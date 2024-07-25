#include <stdlib.h>
#include "../internal/syscall.h"

void _thread_startup(void* arg,void* arg1);

uint64_t __beginthreadex(void* func,size_t stack_size,void* argv) {
    char* stack = (char*)malloc(stack_size + 32) + stack_size;
    return syscall_invoke(syscall_thread_create, (uint64_t)&_thread_startup, (uint64_t)stack, (uint64_t)func,(uint64_t)func);
}

uint64_t __curthread() {
    return syscall_invoke(syscall_gettid,0,0,0,0);
}

uint64_t __endthread(int code) {
    syscall_invoke(syscall_thread_detach, __curthread(),0,0,0); // 先脱离掉线程
    syscall_invoke(syscall_thread_exit,code,0,0,0); // 然后通过exit函数退出
}

uint64_t __jointhread(uint64_t tid) {
    return syscall_invoke(syscall_thread_join, tid,0,0,0);
}

uint64_t __tryjointhread(uint64_t tid) {
    return syscall_invoke(syscall_thread_tryjoin, tid,0,0,0);
}

uint64_t __killthread(uint64_t tid) {
    syscall_invoke(syscall_thread_kill, tid,0,0,0);
}

uint64_t __detachthread(uint64_t tid) {
    syscall_invoke(syscall_thread_detach, tid,0,0,0);
}
