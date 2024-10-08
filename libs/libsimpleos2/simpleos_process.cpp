#include "simpleos_process.h"

#include "syscall.h"

uint64 gettid() {
    return syscall_invoke(syscall_gettid);
}

int64 getuid() {
    return syscall_invoke(syscall_getuid);
}
int64 getgid() {
    return syscall_invoke(syscall_getgid);
}

void thread_waitms(uint64 ms) {
    syscall_invoke(syscall_wait, ms);
}

void thread_create(void (*entry)(void*), void* stack, void* arg) {
    syscall_invoke(syscall_thread_create, (uint64)entry, (uint64)stack, (uint64)arg);
}
void thread_exit(uint64 code) {
    syscall_invoke(syscall_thread_exit, code);
}

void exit(uint64 code) {
    syscall_invoke(syscall_exit,code);
}

int64 thread_movecore(uint64 coreID) {
    return syscall_invoke(syscall_thread_setcore, coreID);
}

uint64 fork() {
    return syscall_invoke(syscall_fork);
}
void exec(const char* path, int argc, const char* const* argv) {
    syscall_invoke(syscall_exec, (uint64)path, (uint64)argc, (uint64)argv);
}

void setfsbase(uint64 val) {
    syscall_invoke(syscall_setfs, val);
}

int64 detach(int64 tid) {
    return syscall_invoke(syscall_thread_detach, tid);
}
int64 join(int64 tid) {
    return syscall_invoke(syscall_thread_join, tid);
}
int64 try_join(int64 tid) {
    return syscall_invoke(syscall_thread_tryjoin, tid);
}
int64 kill(int64 tid) {
    return syscall_invoke(syscall_thread_kill, tid);
}
int64 abort(int64 tid) {
    return syscall_invoke(syscall_abort, tid);
}

void whoami(char* buffer) {
    syscall_invoke(syscall_whoami, (uint64)buffer);
}

int64 changeuser(uint64 uid, uint64 gid) {
    return syscall_invoke(syscall_change_user, uid, gid);
}