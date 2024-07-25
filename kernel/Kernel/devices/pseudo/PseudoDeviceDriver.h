#pragma once

#include "devices/DeviceDriver.h"

/**
 * This driver implements many of the kernel-provided pseudo devices.
 **/
class PseudoDeviceDriver : public CharDeviceDriver {
public:
    /**
     * SubID of the Zero device. 
     * This device returns endless zeroes when read from and discards any data when written to.
     **/
    static constexpr uint64 DeviceZero = 0;
    static constexpr uint64 DeviceNull = 1;
    static constexpr uint64 DeviceDummy0 = 2;
    static constexpr uint64 DeviceDummy1 = 3;

public:
    PseudoDeviceDriver();

    int64 DeviceCommand(uint64 subID, int64 command, void* arg) override;

    uint64 Read(uint64 subID, void* buffer, uint64 bufferSize) override;
    uint64 Write(uint64 subID, const void* buffer, uint64 bufferSize) override;
};