#include "../internal/syscall.h"
#include <fcntl.h>
#include <sys/types.h>

void execve(const char* path, int argc, const char* const* argv,const char** environ) {
    syscall_invoke(syscall_exec, (uint64_t)path, (uint64_t)argc, (uint64_t)argv, (uint64_t)environ);
}

pid_t getpid() {
    return syscall_invoke(syscall_getpid,0,0,0,0);
}

pid_t getppid() {
    return syscall_invoke(syscall_getppid,0,0,0,0);
}

uid_t getuid() {
    return syscall_invoke(syscall_getuid,0,0,0,0);
}

uid_t getgid() {
    return syscall_invoke(syscall_getgid,0,0,0,0);
}
