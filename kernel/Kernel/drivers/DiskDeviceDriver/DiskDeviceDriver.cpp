#include "DiskDeviceDriver.h"
#include "devices/DevFS.h"
#include "stdio.h"

DiskDeviceDriver::DiskDeviceDriver(const char* name) : BlockDeviceDriver(name) {
    this->subIdCount = 0;
}

uint64 DiskDeviceDriver::GetBlockSize(uint64 subID) const {
    for(auto &i : mDriveLists) {
        if(i.subId == subID)
            return i.drive->blksize;
    }
    return 0;
}

void DiskDeviceDriver::RegisterDrive(struct drive_s* drive) {
    uint64 subId = subIdCount++;
    mDriveLists.push_back({
        subId: subId,
        drive: drive
    });
    MakeDiskName(drive->name, drive->type, subId, 0);
    // klog_info("Disk","name=%s",drive->name);
    DevFS::RegisterBlockDevice(drive->name,GetDriverID(),subId);
}

drive_s* DiskDeviceDriver::FindDrive(uint64 subId) {
    for(auto &i : mDriveLists) {
        if(i.subId == subId)
            return i.drive;
    }
    return nullptr;
} 

void DiskDeviceDriver::ScheduleOperation(uint64 subID, uint64 startBlock, uint64 numBlocks, bool write, void* buffer, Atomic<uint64>* finishFlag, Atomic<uint64>* successFlag) {
    drive_s* drive = FindDrive(subID);
    if(drive != nullptr) {
        if(!write) { // Read
            this->DriveReadData(drive,startBlock,numBlocks,buffer,finishFlag,successFlag);
        } else {
            this->DriveWriteData(drive,startBlock,numBlocks,buffer,finishFlag,successFlag);
        }
    }
}

int64 DiskDeviceDriver::DeviceCommand(uint64 subID, int64 command, void* buffer) {
    drive_s* drive = FindDrive(subID);
    if(drive != nullptr) {
        return DriveIoctl(drive, command, buffer);
    }
    return -1;
}