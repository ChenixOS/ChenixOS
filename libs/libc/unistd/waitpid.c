#include "../internal/syscall.h"
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>

pid_t waitpid (pid_t pid, int* statusp, int options)
{
    int64_t ret;
    if(options & WNOHANG) {
        ret = syscall_invoke(syscall_thread_tryjoin,pid,0,0,0);
    } else {
        ret = syscall_invoke(syscall_thread_join,pid,0,0,0);
    }
    if(ret < 0)
        return -1;
    if(statusp)
        *statusp = (int) ret;
    return pid;
}

pid_t wait(int *wstatus)
{
    return waitpid(-1, wstatus, 0);
}