#include "task/Scheduler.h"
#include "syscalls/SyscallFunctions.h"
#include "syscalls/SyscallDefine.h"
#include "arch/MSR.h"
#include "taskctl.h"

#include "klib/errno.h"

int64 ThreadSetUid(uint64 uid) {
    auto tInfo = Scheduler::GetCurrentThreadInfo();

    if(tInfo->uid != 0)
        return ErrorPermissionDenied;

    tInfo->uid = uid;

    return OK;
}

SYSCALL_DEFINE1(syscall_setuid, uint64 uid) {
    return ThreadSetUid(uid);
}

int64 ThreadSetGid(uint64 gid) {
    auto tInfo = Scheduler::GetCurrentThreadInfo();

    if(tInfo->uid != 0)
        return ErrorPermissionDenied;

    tInfo->gid = gid;

    return OK;
}

SYSCALL_DEFINE1(syscall_setgid, uint64 uid) {
    return ThreadSetGid(uid);
}
