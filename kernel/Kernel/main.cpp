#include "global/KernelHeader.h"

#include "terminal/terminal.h"
#include "klib/stdio.h"
#include "arch/GDT.h"
#include "interrupts/IDT.h"
#include "arch/APIC.h"
#include "arch/IOAPIC.h"
#include "syscalls/SyscallHandler.h"
#include "memory/MemoryManager.h"
#include "memory/KernelHeap.h"
#include "multicore/SMP.h"

#include "fs/VFS.h"
#include "fs/tempfs/TempFS.h"
#include "devices/RamDeviceDriver.h"
#include "fs/ext2/ext2.h"
#include "devices/DevFS.h"

#include "task/Scheduler.h"
#include "klib/string.h"

#include "exec/ExecHandler.h"

#include "Config.h"

#include "arch/SSE.h"

#include "klib/errno.h"

#include "syscalls/SyscallDefine.h"

#include "time/Time.h"

#include "init/Init.h"
#include "init/InitInvoke.h"

#include "klib/stdio.h"

#include "multicore/PerCPUInit.h"

#include "acpi/ACPI.h"

#include "klib/GlobalConstructors.h"

extern "C" {
    #include "acpica/acpi.h"
}

#include "kernel/HeaderInfo.h"

static int64 SetupInitProcess(uint64, uint64) {
    uint64 file;
    int64 error = VFS::Open(config_Init_Command, VFS::OpenMode_Read, file);
    if(error != OK) {
        klog_fatal("Init", "Failed to open %s (%s), aborting boot", config_Init_Command, ErrorToString(error));
        return 1;
    }

    VFS::NodeStats stats;
    error = VFS::Stat(config_Init_Command, stats, true);
    if(error != OK) {
        klog_fatal("Init", "Failed to stat %s (%s), aborting boot", config_Init_Command, ErrorToString(error));
        return 1;
    }
    
    uint8* buffer = new uint8[stats.size];
    error = VFS::Read(file, buffer, stats.size);
    if(error < 0) {
        klog_fatal("Init", "Failed to read %s (%s)", config_Init_Command, ErrorToString(error));
        delete[] buffer;
        return 1;
    }
    VFS::Close(file);

    uint64 pml4Entry = MemoryManager::CreateProcessMap();
    IDT::Registers regs;
    if(!ExecHandlerRegistry::Prepare(buffer, stats.size, pml4Entry, &regs, 0, nullptr)) {
        klog_error("Init", "Failed to execute init process, aborting boot");
        MemoryManager::FreeProcessMap(pml4Entry);
        delete[] buffer;
        return 1;
    }

    delete[] buffer;

    klog_info("Init", "Executing init program");
    Scheduler::ThreadExec(pml4Entry, &regs);
    return 1;
}

static KernelHeader* g_KernelHeader;
Terminal::TerminalInfo g_TerminalInfo;

/*
static int64 SpamThread(uint64, uint64) {
    CallInitFuncs(INIT_STAGE_DELAY);
    
    uint64 fd;
    int64 error = VFS::Open("/spam.txt", VFS::OpenMode_Write | VFS::OpenMode_Create | VFS::OpenMode_Clear, fd); 

    VFS::ChangePermissions("/spam.txt", { 3, 1, 0 });

    int i = 0;

    while(true) {
        kfprintf(fd, "Test %i\n", i);
        i++;

        Scheduler::ThreadSleep(1000);
    }
}
*/

static int64 DelayTaskThread(uint64, uint64) {
    CallInitFuncs(INIT_STAGE_DELAY);

    Scheduler::ThreadExit(1);
}

static int64 InitThread(uint64, uint64) {
    klog_info("Boot", "Init KernelThread starting");

    VFS::FileSystem* rootFS = new TempFS();
    VFS::Init(rootFS);

    VFS::FileSystem* devFS = new DevFS();
    int64 error = VFS::CreateFolder("/dev", { 5, 5, 5 });
    if(error < 0) {
        klog_fatal("Init", "Failed to create /dev folder (%s)", ErrorToString(error));
        return 1;
    }
    
    error = VFS::Mount("/dev", devFS);
    if(error < 0) {
        klog_fatal("Init", "Failed to mount DevFS to /dev (%s)", ErrorToString(error));
        return 1;
    }

    CallInitFuncs(INIT_STAGE_DEVDRIVERS);
    CallInitFuncs(INIT_STAGE_FSDRIVERS);
    CallInitFuncs(INIT_STAGE_EXECHANDLERS);
    CallInitFuncs(INIT_STAGE_BUSDRIVERS);

    klog_info("Boot", "All Init function finish.");

    if(g_KernelHeader->ramdiskImage.numPages != 0) {
        RamDeviceDriver* driver = (RamDeviceDriver*)DeviceDriverRegistry::GetDriver("ram");
        driver->AddDevice((char*)g_KernelHeader->ramdiskImage.buffer, 512, g_KernelHeader->ramdiskImage.numPages * 8);
    }

    VConsoleDriver* vcon = (VConsoleDriver*)DeviceDriverRegistry::GetDriver("vconsole");
    vcon->AddConsole(&g_TerminalInfo);

    ACPI::StartSystem();

    kprintf("%C%s\n", 40, 200, 40, config_HelloMessage);

    klog_info("Init", "Mounting boot filesystem");

    error = VFS::CreateFolder("/boot", { 3, 1, 1 });
    if(error < 0) {
        klog_fatal("Init", "Failed to create /boot folder (%s)", ErrorToString(error));
        return 1;
    }
    error = VFS::Mount("/boot", config_BootFS_FSID, config_BootFS_DevFile);
    if(error < 0) {
        klog_fatal("Init", "Failed to mount boot filesystem: %s", ErrorToString(error));
        return 1;
    }

    error = VFS::CreateFolder("/proc", { 1, 1, 1 });
    if(error < 0) {
        klog_fatal("Init", "Failed to create /proc folder (%s)", ErrorToString(error));
        return 1;
    }
    error = VFS::Mount("/proc","procfs");
    if(error < 0) {
        klog_fatal("Init", "Failed to mount proc filesystem: %s", ErrorToString(error));
        return 1;
    }
    
    CallInitFuncs(INIT_STAGE_BUSPROBE);

    uint64 tid = Scheduler::CreateKernelThread(SetupInitProcess);
    // Scheduler::CreateKernelThread(SpamThread);
    Scheduler::CreateKernelThread(DelayTaskThread);

    Time::DateTime dt;
    Time::GetRTC(&dt);

    klog_info("Time", "UTC Time is %02i.%02i.20%02i %02i:%02i:%02i", dt.dayOfMonth, dt.month, dt.year, dt.hours, dt.minutes, dt.seconds);

    while(true) ;

    return 0;
}

extern "C" void __attribute__((noreturn)) main(KernelHeader* info) {
    GlobalConstructors::Run();

    g_KernelHeader = info;

    Terminal::InitTerminalInfo(&g_TerminalInfo, info->screenBuffer, info->screenWidth, info->screenHeight, info->screenScanlineWidth, info->screenColorsInverted);
    Terminal::Clear(&g_TerminalInfo);

    kprintf_isr("%CStarting ChenixOS Kernel\n", 40, 200, 40);
    klog_info_isr("Boot", "Kernel at 0x%016X", info->kernelImage.buffer);

    if(!MemoryManager::Init(info))
        goto bootFailed;
    ACPI::InitEarlyTables(info);
    SMP::GatherInfo();
    PerCPU::Init(SMP::GetCoreCount());
    APIC::Init();
    MemoryManager::InitCore();

    Scheduler::MakeMeIdleThread();

    if(!Time::Init())
        goto bootFailed;
    IOAPIC::Init();
    GDT::Init(SMP::GetCoreCount());
    GDT::InitCore(SMP::GetLogicalCoreID());
    IDT::Init();
    IDT::InitCore(SMP::GetLogicalCoreID());
    APIC::InitBootCore();
    SyscallHandler::Init();
    SyscallHandler::InitCore();
    if(!SSE::InitBootCore())
        goto bootFailed;
    
    
    Scheduler::ThreadEnableInterrupts();
    APIC::StartTimer(10);

    Terminal::EnableDoubleBuffering(&g_TerminalInfo);

    for(int p = 0; p < info->screenBufferPages; p++)
        MemoryManager::EnableWriteCombineOnLargePage((char*)info->screenBuffer + p * 4096);
    

    // TODO: 修复BUG，在VMWare上多核无法启动
    
    SMP::StartCores(info->smpTrampolineBuffer, info->pageBuffer);
    
    MemoryManager::FreePages(MemoryManager::KernelToPhysPtr(info->smpTrampolineBuffer), info->smpTrampolineBufferPages);
    
    HeaderInfo::Init(g_KernelHeader);
    Scheduler::CreateInitKernelThread(InitThread);

    Scheduler::ThreadUnsetSticky();

    while(true)
        __asm__ __volatile__ ("hlt");

bootFailed:
    klog_fatal("Boot", "Boot failed...");
    while(true)
        __asm__ __volatile__ ("hlt");
}