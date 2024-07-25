#pragma once

#include "simpleos_types.h"

// The system call numbers below should be kept in sync with the Kernel's SyscallFunctions.h file

#define NO_USER_SPACE
#include "../../libs/libc/internal/syscall.h"

inline __attribute__((always_inline)) uint64 syscall_invoke(uint64 func, uint64 arg1 = 0, uint64 arg2 = 0, uint64 arg3 = 0, uint64 arg4 = 0) {
    uint64 res;
    register uint64 r8 asm("r8") = arg3;
    register uint64 r9 asm("r9") = arg4;
    __asm__ __volatile__ (
        "syscall"
        : "+D"(func), "+S"(arg1), "+d"(arg2), "+r"(r8), "+r"(r9), "=a"(res)
        : : "rcx", "r10", "r11"
    );
    return res;
}