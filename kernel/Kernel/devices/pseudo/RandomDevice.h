#pragma once

#include "devices/DeviceDriver.h"

/**
 * The driver for '/dev/random'
 **/
class RandomDeviceDriver : public CharDeviceDriver {
public:
    RandomDeviceDriver();

    int64 DeviceCommand(uint64 subID, int64 command, void* arg) override;

    uint64 Read(uint64 subID, void* buffer, uint64 bufferSize) override;
    uint64 Write(uint64 subID, const void* buffer, uint64 bufferSize) override;
};