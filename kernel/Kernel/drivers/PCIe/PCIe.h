#pragma once

#include "types.h"

#include "interrupts/IDT.h"
#include "devices/DeviceDriver.h"

#define PCIE_DEFAULT_VENDORID 0xFFFF
#define PCIE_DEFAULT_DEVICEID 0xFFFF
#define PCIE_DEFAULT_CLASSCODE 0xFF

namespace PCIe {

    struct Device {
        uint16 group;
        uint8 bus;
        uint8 device;
        uint8 function;

        uint16 vendorID;
        uint16 deviceID;

        uint8 classCode;
        uint8 subclassCode;
        uint8 progIf;

        int numBARs;
        uint64 BARs[6];

        uint64 memBase;

        void* msi;
    };

    struct DriverInfo {
        uint16 vendorID;
        uint16 deviceID;

        uint8 classCode;
        uint8 subclassCode;
        uint8 progIf;
    };

    typedef void (*PCIDriverFactory)(const Device& dev);
    void RegisterDriver(const DriverInfo& info, PCIDriverFactory factory);

    void SetInterruptHandler(const Device& dev, IDT::ISR handler);

    uint8 ReadConfigByte(const Device& dev, uint32 reg);
    uint16 ReadConfigWord(const Device& dev, uint32 reg);
    uint32 ReadConfigDWord(const Device& dev, uint32 reg);
    uint64 ReadConfigQWord(const Device& dev, uint32 reg);

    void WriteConfigByte(const Device& dev, uint32 reg, uint8 val);
    void WriteConfigWord(const Device& dev, uint32 reg, uint16 val);
    void WriteConfigDWord(const Device& dev, uint32 reg, uint32 val);
    void WriteConfigQWord(const Device& dev, uint32 reg, uint64 val);

    void WriteConfigMask(const Device& dev, uint32 reg,uint16 off, uint16 on);
    void EnableBusMaster(const Device& dev);
    void EnableMemoryMaster(const Device& dev);

    void* GetBARAddress(const Device& dev,uint8 id);

}