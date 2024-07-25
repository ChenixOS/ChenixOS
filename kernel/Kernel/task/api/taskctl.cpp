#include "task/Scheduler.h"
#include "syscalls/SyscallFunctions.h"
#include "syscalls/SyscallDefine.h"
#include "arch/MSR.h"
#include "taskctl.h"

#include "klib/errno.h"

void ThreadSetFS(uint64 val) {
    Scheduler::GetCurrentThreadInfo()->userFSBase = val;
    MSR::Write(MSR::RegFSBase, val);
}

void ThreadSetGS(uint64 val) {
    Scheduler::GetCurrentThreadInfo()->userGSBase = val;
    MSR::Write(MSR::RegGSBase, val);
}

// ring3层可以通过这些调用实现一些特殊的进程操作
SYSCALL_DEFINE4(syscall_taskctl,uint64 action,uint64 arg1,uint64 arg2,uint64 args) {
    if(action == TASKCTL_SETFS) {
        ThreadSetFS(arg1);
        return OK;
    } else if(action == TASKCTL_SETGS) {
        ThreadSetGS(arg1);
        return OK;
    }
    return ErrorNotSupportOperator;
}

SYSCALL_DEFINE1(syscall_setfs, uint64 val) {
    ThreadSetFS(val);
    return OK;
}
