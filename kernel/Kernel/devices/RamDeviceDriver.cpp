#include "RamDeviceDriver.h"

#include "klib/memory.h"

#include "init/Init.h"

#include "devices/DevFS.h"

static void Init() {
    RamDeviceDriver* driver = new RamDeviceDriver();
}
REGISTER_INIT_FUNC(Init, INIT_STAGE_DEVDRIVERS);

RamDeviceDriver::RamDeviceDriver()
    : BlockDeviceDriver("ram")
{ }

uint64 RamDeviceDriver::AddDevice(char* buffer, uint64 blockSize, uint64 numBlocks) {
    uint64 res = m_Devices.size();
    m_Devices.push_back({ buffer, blockSize, numBlocks });

    DevFS::RegisterBlockDevice("ram0", GetDriverID(), res);

    return res;
}

uint64 RamDeviceDriver::GetBlockSize(uint64 subID) const {
    return m_Devices[subID].blockSize;
}

int64 RamDeviceDriver::DeviceCommand(uint64 subID, int64 command, void* buffer) {
    return OK;
}

void RamDeviceDriver::ScheduleOperation(uint64 subID, uint64 startBlock, uint64 numBlocks, bool write, void* buffer, Atomic<uint64>* finishFlag, Atomic<uint64>* successFlag) {
    const DevInfo& dev = m_Devices[subID];

    if((startBlock + numBlocks) > dev.numBlocks) {
        finishFlag->Write(1);
        successFlag->Write(0);
        return;
    }

    if(write) {
        kmemcpy(dev.buffer + startBlock * GetBlockSize(subID), buffer, GetBlockSize(subID) * numBlocks);
    } else {
        kmemcpy(buffer, dev.buffer + startBlock * GetBlockSize(subID), GetBlockSize(subID) * numBlocks);
    }

    finishFlag->Write(1);
    successFlag->Write(1);
}