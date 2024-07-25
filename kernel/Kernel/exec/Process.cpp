#include "task/Scheduler.h"
#include "syscalls/SyscallHandler.h"

#include "arch/MSR.h"
#include "arch/GDT.h"
#include "klib/stdio.h"
#include "terminal/terminal.h"
#include "klib/string.h"

#include "syscalls/SyscallFunctions.h"
#include "interrupts/IDT.h"
#include "task/Scheduler.h"
#include "memory/MemoryManager.h"
#include "klib/memory.h"

#include "fs/VFS.h"
#include "fs/Permissions.h"

#include "exec/ExecHandler.h"

#include "syscalls/SyscallDefine.h"

namespace Process {
    static uint64 DoFork(SyscallState* state) {
        IDT::Registers childRegs;
        kmemset(&childRegs, 0, sizeof(IDT::Registers));
        childRegs.rip = state->userrip;
        childRegs.userrsp = state->userrsp;
        childRegs.rbp = state->userrbp;
        childRegs.rflags = state->userflags;
        childRegs.rbx = state->userrbx;
        childRegs.r12 = state->userr12;
        childRegs.r13 = state->userr13;
        childRegs.r14 = state->userr14;
        childRegs.r15 = state->userr15;
        childRegs.ss = GDT::UserData;
        childRegs.cs = GDT::UserCode;
        childRegs.ds = GDT::UserData;
        childRegs.rax = 0;

        return Scheduler::CloneThread(false, false, false, &childRegs);
    }
    SYSCALL_DEFINE0(syscall_fork) {
        return DoFork(state);
    }

    static uint64 DoExec(const char* command, int argc, const char* const* argv,const char* const* env) {
        if(!MemoryManager::IsUserPtr(command) || !MemoryManager::IsUserPtr(argv))
            Scheduler::ThreadExit(1);
            
        if(env != nullptr && !MemoryManager::IsUserPtr(env))
            Scheduler::ThreadExit(-1);
        
        ThreadInfo* tInfo = Scheduler::GetCurrentThreadInfo();

        VFS::NodeStats stats;
        int64 error = VFS::Stat(command, stats, true);
        if(error != OK) {
            return error;
        }
        if(tInfo->uid != 0) {
            if(stats.ownerUID == tInfo->uid) {
                if((stats.permissions.ownerPermissions & VFS::Permissions::Execute) == 0)
                    return ErrorPermissionDenied;
            } else if(stats.ownerGID == tInfo->gid) {
                if((stats.permissions.groupPermissions & VFS::Permissions::Execute) == 0)
                    return ErrorPermissionDenied;
            } else {
                if((stats.permissions.otherPermissions & VFS::Permissions::Execute) == 0)
                    return ErrorPermissionDenied;
            }
        }

        bool setUID = false;
        if(stats.permissions.specialFlags & VFS::Permissions::SetUID)
            setUID = true;

        uint64 file;
        error = VFS::Open(command, VFS::OpenMode_Read, file);
        if(error != OK)
            return error;

        uint8* buffer = new uint8[stats.size];
        VFS::Read(file, buffer, stats.size);

        // 进程名字默认继承自命令名
        kstrncpy(tInfo->name, command, 255);

        VFS::FileDescriptor* desc = (VFS::FileDescriptor*) file;
        kstrncpy(tInfo->exe_path, desc->path, 255);

        VFS::Close(file);

        // 处理参数

        char** argPtrBuffer = new char*[argc];
        if(!kmemcpy_usersafe(argPtrBuffer, argv, argc * sizeof(char*))) {
            delete[] buffer;
            delete[] argPtrBuffer;
            Scheduler::ThreadExit(1);
        }

        for(int i = 0; i < argc; i++) {
            char* a = new char[256];
            if(!kpathcpy_usersafe(a, argv[i]))
                Scheduler::ThreadExit(1);

            argPtrBuffer[i] = a;
        }

        // 在这里处理环境信息
        char** envPtrBuffer;
        int envPtrCount;
        if(env == nullptr) {
            envPtrBuffer = new char*[1];
            envPtrCount = 0;
        } else if(*env == nullptr) {
            envPtrBuffer = new char*[1];
            envPtrCount = 0;
        } else {
            int i = 0;
            for(;env[i] != nullptr && i < 128;i++) {}
            envPtrCount = i;
            envPtrBuffer = new char*[envPtrCount];

            for(i = 0;i < envPtrCount;i++){
                char* a = new char[256];
                if(!kpathcpy_usersafe(a, env[i]))
                    Scheduler::ThreadExit(1);
                envPtrBuffer[i] = a;
            }
        }

        uint64 pml4Entry = MemoryManager::CreateProcessMap();
        IDT::Registers regs;
        if(!ExecHandlerRegistry::Prepare(buffer, stats.size, pml4Entry, &regs, argc, argPtrBuffer,envPtrCount,envPtrBuffer)) {
            delete[] buffer;
            MemoryManager::FreeProcessMap(pml4Entry);
            return ErrorInvalidPath;
        }

        delete[] buffer;
        for(int i = 0; i < argc; i++)
            delete[] (argPtrBuffer[i]);
        delete[] argPtrBuffer;

        for(int i = 0;i < envPtrCount;i++)
            delete[] (envPtrBuffer[i]);
        delete[] envPtrBuffer;

        if(setUID) {
            tInfo->uid = stats.ownerUID;
            tInfo->gid = stats.ownerGID;
        }

        Scheduler::ThreadExec(pml4Entry, &regs);
        return 1;
    }
    
    SYSCALL_DEFINE4(syscall_exec, const char* path, int argc, const char* const* argv,const char* const* env) {
        return DoExec(path, argc, argv, env);
    }
}