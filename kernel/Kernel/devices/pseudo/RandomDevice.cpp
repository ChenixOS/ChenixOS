#include "klib/memory.h"
#include "fs/VFS.h"
#include "devices/DevFS.h"

#include "init/Init.h"

#include "RandomDevice.h"
#include "time/Time.h"

#define SEED_FIRST 1476615901
#define SEED_SECOND 18977601

Atomic<uint64> randomSeed;

static void Init() {
    RandomDeviceDriver* driver = new RandomDeviceDriver();
    randomSeed.Write((SEED_FIRST + Time::GetTSC()) * 2 % 10000);
}
REGISTER_INIT_FUNC(Init, INIT_STAGE_DEVDRIVERS);

RandomDeviceDriver::RandomDeviceDriver()
    : CharDeviceDriver("pseudo")
{
    DevFS::RegisterCharDevice("random", GetDriverID(), 1);
    DevFS::RegisterCharDevice("urandom", GetDriverID(), 2);
}

int64 RandomDeviceDriver::DeviceCommand(uint64 subID, int64 command, void* arg) {
    return OK;
}

static uint16 GetRandomSec() {
    uint64 r = ((randomSeed.Read() * SEED_SECOND) + (Time::GetTSC())) * 3 / 2;
    randomSeed++;
    return  (uint16)r;
}

uint64 RandomDeviceDriver::Read(uint64 subID, void* buffer, uint64 bufferSize) {
    if(subID == 1 || subID == 2) {
        char* out = (char*) buffer;
        if(!kmemset_usersafe(buffer, 0, bufferSize))
            return ErrorInvalidBuffer;
        for(uint64 i = 0;i < bufferSize;) {
            if(bufferSize - i >= sizeof(uint16)) {
                *((uint16*)(&out[i])) = GetRandomSec();
                i += sizeof(uint16);
            } else {
                out[i] = (char)GetRandomSec();
                i += 1;
            }
        }
        return bufferSize;
    } else {
        return ErrorInvalidDevice;
    }
}

uint64 RandomDeviceDriver::Write(uint64 subID, const void* buffer, uint64 bufferSize) {
    if(subID == 0) {
        return bufferSize;
    } else {
        return ErrorInvalidDevice;
    }
}