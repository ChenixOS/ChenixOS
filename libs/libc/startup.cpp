#include "internal/ELFProgramInfo.h"
#include "include/stdlib.h"
#include "internal/syscall.h"
#include <stdio.h>
#include <unistd.h>

extern "C" {
    extern int main(int argc, char* argv[]);
}
ELFProgramInfo* g_ProgInfo;

char **environ;

static void setfsbase(uint64 val) {
    syscall_invoke(syscall_setfs, val, 0, 0, 0);
}

char* copy_str(const char* str) {
    if(str == nullptr)
        return nullptr;
    int i = strlen(str);
    char* res = (char*) malloc(i + 1);
    memcpy(res,str,i + 1);
    return res;
}

void copy_env(const char* const* env) {
    if(env == nullptr) {
        void** buf = (void**)malloc(sizeof(void*));
        environ = (char**)buf;
        environ[0] = nullptr;
    } else {
        int i = 0;
        for(i = 0;env[i] != nullptr;i++) {}
        i += 2;
        void** buf = (void**)malloc(sizeof(void*) * i);
        environ = (char**)buf;
        for(i = 0;env[i] != nullptr;i++) {
            environ[i] = copy_str(env[i]);
        }
        environ[i] = nullptr;
    }
}

/* __fastcall */
extern "C" void __start(ELFProgramInfo* info) {
    g_ProgInfo = info;

    // Support for Local TLS
    if(info->masterTLSSize != 0) {
        uint64 allocSize = info->masterTLSSize + sizeof(ELFThread);
        char* alloc = (char*)malloc(allocSize);

        for(int i = 0; i < info->masterTLSSize; i++)
            alloc[i] = info->masterTLSAddress[i];

        ELFThread* thread = (ELFThread*)&alloc[info->masterTLSSize];
        thread->allocPtr = alloc;
        thread->selfPtr = thread;
        thread->progInfo = info;

        setfsbase((uint64)thread);
    }

    copy_env(g_ProgInfo->environ);

    int res = main(g_ProgInfo->argc, g_ProgInfo->argv);
    
    fflush(stdout);

    exit(res);
}

/* __fastcall */
extern "C" void _thread_startup(void* arg,void* arg1) {
    // Support for Local TLS
    if(g_ProgInfo->masterTLSSize != 0) {
        uint64 allocSize = g_ProgInfo->masterTLSSize + sizeof(ELFThread);
        char* alloc = (char*)malloc(allocSize);

        for(int i = 0; i < g_ProgInfo->masterTLSSize; i++)
            alloc[i] = g_ProgInfo->masterTLSAddress[i];

        ELFThread* thread = (ELFThread*)&alloc[g_ProgInfo->masterTLSSize];
        thread->allocPtr = alloc;
        thread->selfPtr = thread;
        thread->progInfo = g_ProgInfo;

        setfsbase((uint64)thread); 
    }

    int (*func)(void*) = (int (*)(void*))arg;

    int res = func(arg1);
}