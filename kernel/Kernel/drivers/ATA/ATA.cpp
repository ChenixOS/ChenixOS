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
    // static IDEDeviceDriver* driver = new IDEDeviceDriver();
    // driver->ScanDevice();
}
REGISTER_INIT_FUNC(Init,INIT_STAGE_BUSDRIVERS);

static StickyLock* ideLock;

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

// 提交请求到PIO设备
static bool
post_ide_request(struct drive_s* drive,struct disk_op_s* op) {
    int sector = op->lba;
    int read_cmd = (op->count == 1) ? IDE_CMD_READ :  IDE_CMD_RDMUL;
    int write_cmd = (op->count == 1) ? IDE_CMD_WRITE : IDE_CMD_WRMUL;
    bool b;

    await_ide(0);
    Port::OutByte(0x3f6, 0);  // generate interrupt
    Port::OutByte(0x1f2, op->count & 0xF);  // number of sectors
    Port::OutByte(0x1f3, sector & 0xff);
    Port::OutByte(0x1f4, (sector >> 8) & 0xff);
    Port::OutByte(0x1f5, (sector >> 16) & 0xff);
    Port::OutByte(0x1f6, 0xe0 | ((drive->cntl_id & 1) << 4) | ((sector >> 24) & 0x0f));

    Time::Delay(1);

    if(op->isWrite) {
        Port::OutByte(0x1f7, write_cmd);
        outsl(0x1f0, (uint32*) op->buf_fl, (op->count * SECTOR_SIZE) / 4);
    } else {
        Port::OutByte(0x1f7, read_cmd);
    }
    Time::Delay(10);
    
    await_ide(0);

    b = (await_ide(1) >= 0);
    if(!op->isWrite && b) {
        insl(0x1f0, (uint32*) op->buf_fl, (op->count * SECTOR_SIZE) / 4);
    }

    return b;
}

// 中断处理程序，从这里开始接收数据
void ide_request_isr(IDT::Registers* regs) {
    klog_info_isr("IDE","IDE IRQ !!!!");
}

// ==============================================================================

static bool
pio_ide_readwrite(struct drive_s* drive,
    bool isWrite,
    uint64 startBlock,
    uint64 numBlock,
    void* buffer) {
        klog_info("IDE","PIO Req: %s %X %X +%X -> %X",isWrite ? "W" : "R",drive,startBlock,numBlock,buffer);

        struct disk_op_s op;
        op.buf_fl = buffer;
        op.lba = startBlock;
        op.count = numBlock;
        op.isWrite = isWrite;
        return post_ide_request(drive, &op);
}

// 默认构造
IDEDeviceDriver::IDEDeviceDriver()
: DiskDeviceDriver("ide") {
}
	
void IDEDeviceDriver::DriveReadData(struct drive_s* drive,uint64 startBlock,uint64 numBlocks,void* buffer, Atomic<uint64>* finishFlag, Atomic<uint64>* successFlag) {
    memset(buffer, 0, drive->blksize * numBlocks);
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


    // TODO: IDE驱动有问题
    /*for(uint32 id = 0; id < 2; id++) {
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
    Port::OutByte(0x1f6, 0xe0 | (0 << 4));*/
}

