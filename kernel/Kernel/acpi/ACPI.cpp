#include "ACPI.h"

#include "memory/MemoryManager.h"
#include "klib/stdio.h"
#include "klib/memory.h"
#include "interrupts/IDT.h"
#include "task/Scheduler.h"
#include "fs/VFS.h"

#include "klib/errno.h"

extern "C" {
    #include "acpica/acpi.h"
}

uint64 g_ACPILogFile;

namespace ACPI {

    static RSDPDescriptor* g_RSDP;
    static XSDT* g_XSDT;
    static MADT* g_MADT;
    static MCFG* g_MCFG;

    void PrintAllXSDTTable() {
        uint32 numEntries = (g_XSDT->header.length - sizeof(SDTHeader)) / 8;
        char name[5];
        kmemset(name,0,5);
        for(int i = 0; i < numEntries; i++) {
            SDTHeader* entry = (SDTHeader*)MemoryManager::PhysToKernelPtr(g_XSDT->tables[i]);
            kmemcpy(name,&entry->signature,4);
            klog_info_isr("ACPI", "Found Table: %s",&name);
        }
    }

    void InitEarlyTables(KernelHeader* header) {
        auto rsdp = static_cast<RSDPDescriptor*>(header->rsdp);
        auto xsdt = static_cast<XSDT*>(MemoryManager::PhysToKernelPtr((void*)rsdp->xsdtAddress));
        g_RSDP = rsdp;
        g_XSDT = xsdt;

        #ifdef ACTION_DEBUG_ACPI_TABLE
        PrintAllXSDTTable();
        #endif
        
        auto mcfg = reinterpret_cast<MCFG*>(xsdt->FindTable(SIGNATURE('M', 'C', 'F', 'G')));
        if(mcfg == nullptr) {
            klog_info_isr("ACPI", "Could not retrieve MCFG table");
            // while(true);
        }

        auto madt = reinterpret_cast<MADT*>(xsdt->FindTable(SIGNATURE('A', 'P', 'I', 'C')));
        if(madt == nullptr) {
            klog_fatal_isr("ACPI", "Could not retrieve MADT table");
            while(true);
        }

        g_MADT = madt;
        g_MCFG = mcfg;
    }

    static ACPI_BUFFER RunAcpi(ACPI_HANDLE objHandle, const char* method, ACPI_OBJECT* args, uint32 argCount);
    static void FreeBuffer(const ACPI_BUFFER& buffer);
    static void AcpiEventHandler(UINT32 eventType, ACPI_HANDLE dev, UINT32 eventNumber, void* arg);

    extern "C" int64 AcpiJobThread(uint64, uint64);

    bool StartSystem() {
        int64 error = VFS::Open("/acpi.log", VFS::OpenMode_Create | VFS::OpenMode_Clear | VFS::OpenMode_Write, g_ACPILogFile);
        if(error != OK) {
            klog_fatal("ACPI", "Failed to open ACPI log file (/acpi.log): %s", ErrorToString(error));
            return false;
        }

        ACPI_STATUS err;
        err = AcpiInitializeSubsystem();
        if(err != AE_OK) {
            kfprintf(g_ACPILogFile, "Failed to init ACPI: %i\n", err);
            return false;
        }
        err = AcpiInitializeTables(nullptr, 16, false);
        if(err != AE_OK) {
            kfprintf(g_ACPILogFile, "Failed to init ACPI tables: %i\n", err);
            return false;
        }
        err = AcpiLoadTables();
        if(err != AE_OK) {
            kfprintf(g_ACPILogFile, "Failed to load ACPI tables: %i\n", err);
            return false;
        }

        err = AcpiEnableSubsystem(ACPI_FULL_INITIALIZATION);
        if(err != AE_OK) {
            kfprintf(g_ACPILogFile, "Failed to enter ACPI mode: %i\n", err);
            return false;
        }
        err = AcpiInitializeObjects(ACPI_FULL_INITIALIZATION);
        if(err != AE_OK) {
            kfprintf(g_ACPILogFile, "Failed to init ACPI Objects: %i\n", err);
            return false;
        }

        err = AcpiUpdateAllGpes();
        if(err != AE_OK) {
            kfprintf(g_ACPILogFile, "Failed to update GPEs: %i\n", err);
            return false;
        }

        err = AcpiInstallGlobalEventHandler(AcpiEventHandler, nullptr);
        if(err != AE_OK) {
            kfprintf(g_ACPILogFile, "Failed to install power button handler: %i\n", err);
            return false;
        }
        err = AcpiEnableEvent(ACPI_EVENT_POWER_BUTTON, 0);
        if(err != AE_OK) {
            kfprintf(g_ACPILogFile, "Failed to enable power button handler: %i\n", err);
            return false;
        }

        ACPI_OBJECT arg;
        arg.Type = ACPI_TYPE_INTEGER;
        arg.Integer.Value = 1;
        auto ret = RunAcpi(nullptr, "\\_PIC", &arg, 1);
        FreeBuffer(ret);

        auto tid = Scheduler::CreateKernelThread(AcpiJobThread);
        klog_info("ACPI", "Created Job Thread with TID %i", tid);

        klog_info("ACPI", "Entered ACPI mode");

        return true;
    }

    const MADT* GetMADT() {
        return g_MADT;
    }
    const MCFG* GetMCFG() {
        return g_MCFG;
    }
    const RSDPDescriptor* GetRSDP() {
        return g_RSDP;
    }

    SDTHeader* XSDT::FindTable(uint32 signature) const {
        uint32 numEntries = (header.length - sizeof(XSDT)) / 8;
        for(int i = 0; i < numEntries; i++) {
            SDTHeader* entry = (SDTHeader*)MemoryManager::PhysToKernelPtr(tables[i]);
            if(entry->signature == signature)
                return entry;
        }
        return nullptr;
    }

    MADTEntryHeader* MADT::GetNextEntry(uint64& pos) const {
        if(pos >= header.length - sizeof(MADT))
            return nullptr;

        MADTEntryHeader* entry = (MADTEntryHeader*)&entries[pos];
        pos += entry->length;
        return entry;
    }

    static void ShutdownJob(void* arg) {
        klog_warning("ACPI", "Power button pressed, shutting down in 3 seconds...");

        Scheduler::ThreadSleep(3000);

        AcpiEnterSleepStatePrep(ACPI_STATE_S5);
        IDT::DisableInterrupts();
        AcpiEnterSleepState(ACPI_STATE_S5);
    }

    static void AcpiEventHandler(UINT32 eventType, ACPI_HANDLE dev, UINT32 eventNumber, void* arg) {
        if(eventType == ACPI_EVENT_TYPE_FIXED && eventNumber == ACPI_EVENT_POWER_BUTTON) {
            AcpiOsExecute(OSL_GPE_HANDLER, ShutdownJob, nullptr);
            AcpiClearEvent(ACPI_EVENT_POWER_BUTTON);
        }
    }

    static ACPI_BUFFER RunAcpi(ACPI_HANDLE objHandle, const char* method, ACPI_OBJECT* args, uint32 argCount) {
        ACPI_BUFFER retVal = { ACPI_ALLOCATE_BUFFER, nullptr };
        ACPI_OBJECT_LIST argList = { argCount, args };
        ACPI_STATUS err = AcpiEvaluateObject(objHandle, (char*)method, &argList, &retVal);
        if(err != AE_OK) {
            kfprintf(g_ACPILogFile, "Failed to execute %s: %i\n", method, err);
        }
        return retVal;
    }

    static void FreeBuffer(const ACPI_BUFFER& buffer) {
        if(buffer.Pointer != nullptr)
            AcpiOsFree(buffer.Pointer);
    }

    static ACPI_STATUS SearchPCIRootBridge (ACPI_HANDLE dev, UINT32 nestLevel, void* ctx, void** retVal) {
        ACPI_DEVICE_INFO* info;
        AcpiGetObjectInfo(dev, &info);

        if(info->Flags & ACPI_PCI_ROOT_BRIDGE) {
            *retVal = dev;
            ACPI_FREE(info);
            return -1;
        }

        ACPI_FREE(info);
        return AE_OK;
    }

    Handle GetPCIRootBridge() {
        void* retVal;
        ACPI_STATUS err = AcpiGetDevices(nullptr, SearchPCIRootBridge, nullptr, &retVal);
        if(err != -1) {
            klog_error("ACPI", "Failed to find PCI Root Bridge: %i", err);
            return nullptr;
        }

        return retVal;
    }

    bool GetPCIDeviceIRQ(uint8 devID, uint8 pin, AcpiIRQInfo& irqInfo) {
        auto rootBridge = GetPCIRootBridge();
        if(rootBridge == nullptr) {
            klog_error("ACPI", "GetPCIDeviceIRQ: Failed to find PCI root bridge");
            return false;
        }

        ACPI_BUFFER retBuffer = { ACPI_ALLOCATE_BUFFER, nullptr };
        ACPI_STATUS err = AcpiGetIrqRoutingTable(rootBridge, &retBuffer);
        if(err != AE_OK) {
            klog_error("ACPI", "GetPCIDeviceIRQ: Failed to retrieve routing table of root bridge: %i", err);
            FreeBuffer(retBuffer);
            return false;
        }

        ACPI_PCI_ROUTING_TABLE* entry = nullptr;

        auto routeTable = (ACPI_PCI_ROUTING_TABLE*)retBuffer.Pointer;
        while(routeTable->Length != 0) {
            uint16 entryDevID = (routeTable->Address >> 16) & 0xFFFF;
            if(entryDevID == devID && routeTable->Pin == pin) {
                entry = routeTable;
                break;
            }

            routeTable = (ACPI_PCI_ROUTING_TABLE*)((char*)routeTable + routeTable->Length);
        }

        if(entry == nullptr) {
            klog_error("ACPI", "GetPCIDeviceIRQ: Failed to find routing table entry for device %i", devID);
            FreeBuffer(retBuffer);
            return false;
        }

        if(entry->Source[0] == '\0') {
            irqInfo.isGSI = true;
            irqInfo.number = entry->SourceIndex;
            FreeBuffer(retBuffer);
            return true;
        } else {
            ACPI_HANDLE dev;
            err = AcpiGetHandle(nullptr, entry->Source, &dev);
            if(err != AE_OK) {
                klog_error("ACPI", "GetPCIDeviceIRQ: Failed to find Link device %s: %i", entry->Source, err);
                FreeBuffer(retBuffer);
                return false;
            }

            uint32 index = entry->SourceIndex;

            FreeBuffer(retBuffer);

            retBuffer = RunAcpi(dev, "_CRS", nullptr, 0);
            if(retBuffer.Pointer == nullptr) {
                klog_error("ACPI", "GetPCIDeviceIRQ: Failed to execute _CRS method on link device");
                return false;
            }

            char* entryPtr = (char*)retBuffer.Pointer;
            for(int i = 0; i < index; i++) {
                if(entryPtr[0] & 0x80) {
                    entryPtr += ((int16)entryPtr[2] << 16) | (int16)entryPtr[1];
                } else {
                    entryPtr += entryPtr[0] & 0x7;
                }
            }

            if(entryPtr[0] & 0xFE == 0x22) {
                uint16 mask = *(uint16*)(&entryPtr[1]);
                for(int i = 0; i < 16; i++) {
                    if(mask & (1 << i)) {
                        irqInfo.isGSI = false;
                        irqInfo.number = i;
                        FreeBuffer(retBuffer);
                        return true;
                    }
                }
            } else if(entryPtr[0] == 0x89) {
                irqInfo.isGSI = false;
                irqInfo.number = *(uint32*)&entryPtr[5];
                FreeBuffer(retBuffer);
                return true;
            }

            klog_error("ACPI", "GetPCIDeviceIRQ: Unsupported Resource descriptor type: %02X", entryPtr[0]);
            FreeBuffer(retBuffer);
            return false;
        }
    }

}