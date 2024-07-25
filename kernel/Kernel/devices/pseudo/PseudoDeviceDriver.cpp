#include "PseudoDeviceDriver.h"

#include "klib/memory.h"
#include "fs/VFS.h"
#include "devices/DevFS.h"

#include "init/Init.h"

static void Init() {
    PseudoDeviceDriver* driver = new PseudoDeviceDriver();
}
REGISTER_INIT_FUNC(Init, INIT_STAGE_DEVDRIVERS);

PseudoDeviceDriver::PseudoDeviceDriver()
    : CharDeviceDriver("pseudo")
{
    DevFS::RegisterCharDevice("zero", GetDriverID(), DeviceZero);
    DevFS::RegisterCharDevice("null",GetDriverID(),DeviceNull);
    DevFS::RegisterCharDevice("dummy0",GetDriverID(),DeviceDummy0);
    DevFS::RegisterCharDevice("dummy1",GetDriverID(),DeviceDummy1);
}

int64 PseudoDeviceDriver::DeviceCommand(uint64 subID, int64 command, void* arg) {
    return OK;
}

uint64 PseudoDeviceDriver::Read(uint64 subID, void* buffer, uint64 bufferSize) {
    if(subID == DeviceZero || subID == DeviceDummy0 || subID == DeviceDummy1) {
        if(!kmemset_usersafe(buffer, 0, bufferSize))
            return ErrorInvalidBuffer;
        return bufferSize;
    } else if(subID == DeviceNull) {
        return 0;
    } else {
        return ErrorInvalidDevice;
    }
}

uint64 PseudoDeviceDriver::Write(uint64 subID, const void* buffer, uint64 bufferSize) {
    if(subID == DeviceZero || subID == DeviceDummy0 || subID == DeviceDummy1) {
        return bufferSize;
    } else if(subID == DeviceNull) {
        return 0;
    } else {
        return ErrorInvalidDevice;
    }
}