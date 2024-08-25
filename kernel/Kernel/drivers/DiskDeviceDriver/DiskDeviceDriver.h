#pragma once

#include <stdint.h>

struct disk_op_s {
    void *buf_fl; // buffer
    struct drive_s *drive_fl; // 指向设备方便管理
    uint16 count;
    bool isWrite;
    union {
        // Commands: READ, WRITE, VERIFY, SEEK, FORMAT
        uint64 lba;
        // Commands: SCSI
        struct {
            uint16 blocksize;
            void *cdbcmd;
        };
    };
};

#define CMD_RESET   0x00
#define CMD_READ    0x02
#define CMD_WRITE   0x03
#define CMD_VERIFY  0x04
#define CMD_FORMAT  0x05
#define CMD_SEEK    0x07
#define CMD_ISREADY 0x10
#define CMD_SCSI    0x20


/****************************************************************
 * Global storage
 ****************************************************************/

struct chs_s {
    uint16 head;
    uint16 cylinder;
    uint16 sector;
    uint16 pad;
};

struct drive_s {
    uint8 type;            // Driver type (DTYPE_*)
    char name[10];
    
    uint32 cntl_id;        // Unique id for a given driver type.
    uint8 removable;       // Is media removable (currently unused)

    uint64 sectors;        // Total sectors count
    uint16 blksize;        // block size

    struct chs_s lchs;  // Logical CHS
    
    struct chs_s pchs;  // Physical CHS
    uint32 max_segment_size; //max_segment_size
    uint32 max_segments;   //max_segments
};

#define DISK_SECTOR_SIZE  512
#define CDROM_SECTOR_SIZE 2048

#define DTYPE_NONE         0x00
#define DTYPE_FLOPPY       0x10
#define DTYPE_ATA          0x20
#define DTYPE_ATA_ATAPI    0x21
#define DTYPE_RAMDISK      0x30
#define DTYPE_CDEMU        0x40
#define DTYPE_AHCI         0x50
#define DTYPE_AHCI_ATAPI   0x51
#define DTYPE_VIRTIO_SCSI  0x60
#define DTYPE_VIRTIO_BLK   0x61
#define DTYPE_USB          0x70
#define DTYPE_USB_32       0x71
#define DTYPE_UAS          0x72
#define DTYPE_UAS_32       0x73
#define DTYPE_LSI_SCSI     0x80
#define DTYPE_ESP_SCSI     0x81
#define DTYPE_MEGASAS      0x82
#define DTYPE_PVSCSI       0x83
#define DTYPE_MPT_SCSI     0x84
#define DTYPE_SDCARD       0x90
#define DTYPE_NVME         0x91

#define MAXDESCSIZE 80

#define TRANSLATION_NONE  0
#define TRANSLATION_LBA   1
#define TRANSLATION_LARGE 2
#define TRANSLATION_RECHS 3
#define TRANSLATION_HOST 4

#define EXTTYPE_FLOPPY 0
#define EXTTYPE_HD 1
#define EXTTYPE_CD 2

#define EXTSTART_HD 0x80
#define EXTSTART_CD 0xE0

#define DISK_RET_SUCCESS       0x00
#define DISK_RET_EPARAM        0x01
#define DISK_RET_EADDRNOTFOUND 0x02
#define DISK_RET_EWRITEPROTECT 0x03
#define DISK_RET_ECHANGED      0x06
#define DISK_RET_EBOUNDARY     0x09
#define DISK_RET_EBADTRACK     0x0c
#define DISK_RET_ECONTROLLER   0x20
#define DISK_RET_ETIMEOUT      0x80
#define DISK_RET_ENOTLOCKED    0xb0
#define DISK_RET_ELOCKED       0xb1
#define DISK_RET_ENOTREMOVABLE 0xb2
#define DISK_RET_ETOOMANYLOCKS 0xb4
#define DISK_RET_EMEDIA        0xC0
#define DISK_RET_ENOTREADY     0xAA

#include "devices/DeviceDriver.h"
#include <vector>

class DiskDeviceDriver : public BlockDeviceDriver {
private:
    struct DriveList {
        uint64 subId;
        struct drive_s* drive;
    };

    uint64 subIdCount;
    std::vector<struct DriveList> mDriveLists;

    drive_s* FindDrive(uint64 subId);
public:
    DiskDeviceDriver(const char* name);

    uint64 GetBlockSize(uint64 subID) const override;

    int64 DeviceCommand(uint64 subID, int64 command, void* buffer) override;

    void RegisterDrive(struct drive_s* drive);
    virtual void DriveReadData(struct drive_s* drive,uint64 startBlock,uint64 numBlocks,void* buffer, Atomic<uint64>* finishFlag, Atomic<uint64>* successFlag) = 0;
    virtual void DriveWriteData(struct drive_s* drive,uint64 startBlock,uint64 numBlocks,void* buffer, Atomic<uint64>* finishFlag, Atomic<uint64>* successFlag) = 0;
    virtual int64 DriveIoctl(struct drive_s* drive,int64 cmd,void* buffer) = 0;
protected:
    void ScheduleOperation(uint64 subID, uint64 startBlock, uint64 numBlocks, bool write, void* buffer, Atomic<uint64>* finishFlag, Atomic<uint64>* successFlag) override;
};

inline void MakeDiskName(char* out,uint8 type,uint8 count,uint8 vcount) {
    int i = 0;
    if(type == DTYPE_ATA) {
        out[i++] = 'h';
    } else if(type == DTYPE_AHCI) {
        out[i++] = 's';
    } else if(type == DTYPE_AHCI_ATAPI) {
        out[i++] = 'c';
        out[i++] = 'd';
    } else if(type == DTYPE_VIRTIO_BLK) {
        out[i++] = 'v';
    } else if(type == DTYPE_NVME) {
        out[i++] = 'n';
    } else {
        out[i++] = 'r';
    }
    out[i++] = 'd';

    out[i++] = ('a' + count);

    if(vcount > 0) {
        out[i++] = ('0' + vcount);
    }

    out[i++] = 0;
}

static inline void make_disk_op(struct drive_s* driver,
    struct disk_op_s* op,
    uint64 startBlock,
    uint16 numBlock,
    void* buffer) {
    op->buf_fl = buffer;
    op->blocksize = driver->blksize;
    op->lba = startBlock;
    op->count = numBlock;
}