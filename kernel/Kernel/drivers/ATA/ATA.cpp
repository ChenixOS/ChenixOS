/**
 * @file ATA.cpp
 * @author liankong xhsw.new@qq.com
 * @brief Low level driver for ATA/IDE device
 * @version 0.1
 * @date 2024-08-13
 * 
 * @copyright Copyright (c) 2024
 * 
 */
#include "ATA.h"
#include "init/Init.h"
#include "drivers/PCI/PCI.h"
#include "devices/DevFS.h"
#include "klib/stdio.h"
#include "klib/memory.h"
#include "memory/KernelHeap.h"
#include "time/Time.h"
#include "task/Scheduler.h"
#include "drivers/ATA/ATA.h"
#include "impl/lowbits.h"
#include "impl/pcireg.h"
#include "arch/Port.h"
#include "arch/IOAPIC.h"
#include "stl/fifo.h"

#define SECTOR_SIZE   512
#define IDE_BSY       0x80
#define IDE_DRDY      0x40
#define IDE_DF        0x20
#define IDE_ERR       0x01

#define IDE_CMD_READ  0x20
#define IDE_CMD_WRITE 0x30
#define IDE_CMD_RDMUL 0xc4
#define IDE_CMD_WRMUL 0xc5

// 初始化
static void Init() {
    static IDEDeviceDriver* driver = new IDEDeviceDriver();
    driver->ScanDevice();
}
REGISTER_INIT_FUNC(Init,INIT_STAGE_BUSDRIVERS);

static StickyLock* ideLock;
static FIFOBuffer<struct disk_op_s *> ide_queue(0);
static volatile int block_flags = 0; // 额外一层保证

// Wait for IDE disk to become ready.
static int
await_ide(int checkerr)
{
  int r;

  while(((r = Port::InByte(0x1f7)) & (IDE_BSY | IDE_DRDY)) != IDE_DRDY) {
    // Scheduler::Yield();
  }
  if(checkerr && (r & (IDE_DF | IDE_ERR)) != 0)
    return -1;
  return 0;
}

// Wait for block flags (more safely)
static void
await_block_flags()
{
    volatile int* block_flags_ptr = &block_flags;

    barrier();
    for(;block_flags || *block_flags_ptr;) {
        barrier();
        Scheduler::Yield();
    }
}

// 提交请求到PIO设备
static void
post_ide_request(struct drive_s* drive,struct disk_op_s* op) {
    int sector = op->lba;
    int read_cmd = (op->count == 1) ? IDE_CMD_READ :  IDE_CMD_RDMUL;
    int write_cmd = (op->count == 1) ? IDE_CMD_WRITE : IDE_CMD_WRMUL;

    await_ide(0);
    Port::OutByte(0x3f6, 0);  // generate interrupt
    Port::OutByte(0x1f2, op->count & 0xF);  // number of sectors
    Port::OutByte(0x1f3, sector & 0xff);
    Port::OutByte(0x1f4, (sector >> 8) & 0xff);
    Port::OutByte(0x1f5, (sector >> 16) & 0xff);
    Port::OutByte(0x1f6, 0xe0 | ((drive->cntl_id & 1) << 4) | ((sector >> 24) & 0x0f));

    if(op->isWrite){
        Port::OutByte(0x1f7, write_cmd);
        outsl(0x1f0, (uint32*) op->buf_fl, (op->count * SECTOR_SIZE) / 4);
    } else {
        Port::OutByte(0x1f7, read_cmd);
    }
}

// 中断处理程序，从这里开始接收数据
void ide_request_isr(IDT::Registers* regs) {
    struct disk_op_s* op;

    klog_info_isr("IDE","IDE IRQ !!!!");

    barrier();
    block_flags = 1;

    bool hasReq = ide_queue.pop(op);
    if(!hasReq)
        goto finish;
    
    if (!op->isWrite && await_ide(1) >= 0) {
        insl(0x1f0, (uint32 *)op->buf_fl, (op->count * SECTOR_SIZE) / 4);
        op->isSuccess = true;
    } else {
        op->isSuccess = false;
    }
    op->isFinish = true;

finish:
    barrier();
    block_flags = 0;
}

static void
await_disk_op_finish(struct disk_op_s* op) {
    barrier();
    for(; !op->isFinish;) {
        barrier();
        Scheduler::Yield();
    }
}

// ==============================================================================

static bool
pio_ide_readwrite(struct drive_s* drive,
    bool isWrite,
    uint64 startBlock,
    uint64 numBlock,
    void* buffer) {
        struct disk_op_s op;
        op.isWrite = isWrite;
        op.isSuccess = false;
        op.buf_fl = buffer;
        op.count = numBlock;
        op.lba = startBlock;

        await_block_flags();
        ideLock->Spinlock();
        await_block_flags();
        
        ide_queue.push(&op);
        post_ide_request(drive, &op);

        klog_info("IDE","Wait for finish.");
        await_disk_op_finish(&op);

        ideLock->Unlock();

        if(!isWrite && !op.isSuccess) {
            return false;
        }
        return true;
}

// 默认构造
IDEDeviceDriver::IDEDeviceDriver()
: DiskDeviceDriver("ide") {
}
	
void IDEDeviceDriver::DriveReadData(struct drive_s* drive,uint64 startBlock,uint64 numBlocks,void* buffer, Atomic<uint64>* finishFlag, Atomic<uint64>* successFlag) {
    bool b = pio_ide_readwrite(drive,false,startBlock,numBlocks,buffer);
    successFlag->Write(b);
    finishFlag->Write(1);
}
void IDEDeviceDriver::DriveWriteData(struct drive_s* drive,uint64 startBlock,uint64 numBlocks,void* buffer, Atomic<uint64>* finishFlag, Atomic<uint64>* successFlag){
    pio_ide_readwrite(drive,true,startBlock,numBlocks,buffer);
    successFlag->Write(1);
    finishFlag->Write(1);
}
int64 IDEDeviceDriver::DriveIoctl(struct drive_s* drive,int64 cmd,void* buffer) {}

void IDEDeviceDriver::ScanDevice() {
    int i;

    ideLock = new StickyLock();
    await_ide(0);
    IOAPIC::RegisterIRQ(14, ide_request_isr);

    for(uint32 id = 0; id < 2; id++) {
        // Check if disk 1 is present
        Port::OutByte(0x1f6, 0xe0 | (id << 4));
        Time::Delay(5);
        for(i = 0; i < 1000; i++) {
            if(Port::InByte(0x1f7) != 0) {
                klog_info("IDE", "founded PIO IDE Device: %d", id);
                struct drive_s* drive = new drive_s;
                drive->type = DTYPE_ATA;
                drive->cntl_id = id;
                drive->blksize = SECTOR_SIZE;
                this->RegisterDrive(drive);
                break;
            }
        }
    }
    // Switch back to disk 0.
    Port::OutByte(0x1f6, 0xe0 | (0 << 4));
}

