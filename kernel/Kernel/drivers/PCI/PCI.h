#pragma once

#include "types.h"

#include "interrupts/IDT.h"
#include "devices/DeviceDriver.h"

#define PCIE_DEFAULT_VENDORID 0xFFFF
#define PCIE_DEFAULT_DEVICEID 0xFFFF
#define PCIE_DEFAULT_CLASSCODE 0xFF

namespace PCI {

    struct Device {
        uint16 group;
        uint16 bus;
        uint16 device;
        uint16 function;

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

    const PCI::Device* GetDeviceByInfo(uint16 group,uint16 bus,uint16 deviceId,uint16 function);
}


namespace IoBase {
    static inline uint8 ReadByte(void* iobase,uint32 reg) {
        barrier();
        return *((uint8*)iobase + reg);
    }

    static inline uint16 ReadWord(void* iobase,uint32 reg) {
        barrier();
        return *(uint16*)((uint8*)iobase + reg);
    }

    static inline uint32 ReadDWord(void* iobase,uint32 reg) {
        barrier();
        return *(uint32*)((uint8*)iobase + reg);
    }

    static inline uint64 ReadQWord(void* iobase,uint32 reg) {
        barrier();
        return *(uint64*)((uint8*)iobase + reg);
    }

    static inline void WriteByte(void* iobase,uint32 reg,uint8 val) {
        barrier();
        *((uint8*)iobase + reg) = val;
    }

    static inline void WriteWord(void* iobase,uint32 reg,uint16 val) {
        barrier();
        *(uint16*)((uint8*)iobase + reg) = val;
    }

    static inline void WriteDWord(void* iobase,uint32 reg,uint32 val) {
        barrier();
        *(uint32*)((uint8*)iobase + reg) = val;
    }

    static inline void WriteQWord(void* iobase,uint32 reg,uint64 val) {
        barrier();
        *(uint64*)((uint8*)iobase + reg) = val;
    }
};