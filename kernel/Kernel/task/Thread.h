#pragma once

#include "types.h"
#include "interrupts/IDT.h"
#include "multicore/Atomics.h"
#include "multicore/locks/StickyLock.h"
#include "klib/AnchorList.h"
#include "syscalls/SyscallDefine.h"

#include <vector>

constexpr uint64 KernelStackPages = 3;
constexpr uint64 KernelStackSize = KernelStackPages * 4096;
constexpr uint64 UserStackPages = 4;
constexpr uint64 UserStackSize = UserStackPages * 4096;

struct ThreadInfo;

struct ThreadState {
    enum Type {
        READY,

        SLEEP,
        QUEUE_LOCK,
        JOIN,

        FINISHED,
        EXITED,
    } type;

    uint64 arg;
};

struct ThreadMemSpace {
    Atomic<uint64> refCount;
    uint64 pml4Entry;
};

struct ThreadFileDescriptor {
    int64 id;
    uint64 sysDesc;
};

struct ThreadFileDescriptors {
    Atomic<uint64> refCount;
    StickyLock lock;
    std::vector<ThreadFileDescriptor> fds;
};

struct ThreadInfo {
    ktl::Anchor<ThreadInfo> activeListAnchor;
    ktl::Anchor<ThreadInfo> globalListAnchor;
    ktl::Anchor<ThreadInfo> joinListAnchor;

    ThreadInfo* mainThread;                     // never changes, safe to access lockless

    StickyLock childThreadsLock;
    ktl::AnchorList<ThreadInfo, &ThreadInfo::joinListAnchor> childThreads;

    int64 tid;
    int64 pid;
    int64 ppid;
    uint64 uid;
    uint64 gid;

    int64 exitCode;

    char name[256];
    char exe_path[256]; // 执行文件的路径
    char cwd[256];

    bool killPending;                       // Set this flag to inform a thread that it should kill itself
    uint64 killHandlerRip;
    uint64 killHandlerRsp;
    uint64 killHandlerSignum;
    IDT::Registers killHandlerReturnState;

    bool abortPending;

    ThreadMemSpace* memSpace;
    ThreadFileDescriptors* fds;

    ThreadState state;
    
    uint64 kernelStack;
    uint64 userGSBase;
    uint64 userFSBase;

    uint64 stickyCount;
    uint64 cliCount;

    uint64 faultRip;

    IDT::Registers registers;

    char* fpuBuffer;   // buffer used to save and restore the SSE and FPU state of the thread
};