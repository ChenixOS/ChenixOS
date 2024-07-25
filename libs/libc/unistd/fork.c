#include "../internal/syscall.h"
#include <fcntl.h>
#include <sys/types.h>

pid_t fork() {
    return syscall_invoke(syscall_fork,0,0,0,0);
}