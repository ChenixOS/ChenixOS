#include "SyscallHandler.h"

#include "arch/MSR.h"
#include "arch/GDT.h"
#include "klib/stdio.h"
#include "terminal/terminal.h"
#include "klib/string.h"

#include "SyscallFunctions.h"
#include "interrupts/IDT.h"
#include "task/Scheduler.h"
#include "memory/MemoryManager.h"
#include "klib/memory.h"

#include "fs/VFS.h"
#include "fs/Permissions.h"

#include "exec/ExecHandler.h"

#include "SyscallDefine.h"

namespace SyscallHandler {

    extern "C" void SyscallEntry();

    extern "C" int SYSCALL_ARRAY_START;
    extern "C" int SYSCALL_ARRAY_END;

    typedef uint64 (*SyscallFunc)(uint64 arg1, uint64 arg2, uint64 arg3, uint64 arg4, SyscallState* state);

    static SyscallFunc g_Functions[1024] = { nullptr };

    void Init() {
        uint64* arr = (uint64*)&SYSCALL_ARRAY_START;
        uint64* arrEnd = (uint64*)&SYSCALL_ARRAY_END;
        for(; arr != arrEnd; arr += 2) {
            g_Functions[arr[1]] = (SyscallFunc)(arr[0]);
        }
    }

    void InitCore()
    {
        uint64 eferVal = MSR::Read(MSR::RegEFER);
        eferVal |= 1;
        MSR::Write(MSR::RegEFER, eferVal);

        uint64 starVal = ((uint64)(GDT::UserCode - 16) << 48) | ((uint64)GDT::KernelCode << 32);
        MSR::Write(MSR::RegSTAR, starVal);

        uint64 lstarVal = (uint64)&SyscallEntry;
        MSR::Write(MSR::RegLSTAR, lstarVal);

        uint64 cstarVal = 0;
        MSR::Write(MSR::RegCSTAR, cstarVal);

        uint64 sfmaskVal = 0b000000000001000000000;     // disable interrupts on syscall
        MSR::Write(MSR::RegSFMASK, sfmaskVal);
    }

    extern "C" uint64 SyscallDispatcher(uint64 func, uint64 arg1, uint64 arg2, uint64 arg3, uint64 arg4, SyscallState* state)
    {
        uint64 res = 0;

        if(g_Functions[func] != nullptr) {
            res = g_Functions[func](arg1, arg2, arg3, arg4, state);
        } else {
            klog_error("Fault", "Thread %i called invalid syscall %i", Scheduler::ThreadGetTID(), func);
            Scheduler::ThreadExit(1);
        }

        Scheduler::ThreadCheckFlags(state, res);
        return res;
    }

}