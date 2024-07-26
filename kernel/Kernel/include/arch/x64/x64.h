#ifndef _ARCH_X64_H_
#define _ARCH_X64_H_

#include <stdint.h>

static inline void nop(void)
{
    asm volatile("nop");
}

static inline void hlt(void)
{
    asm volatile("hlt": : :"memory");
}

#define CPUID_TSC (1 << 4)
#define CPUID_MSR (1 << 5)
#define CPUID_APIC (1 << 9)
#define CPUID_MTRR (1 << 12)
#define CPUID_X2APIC (1 << 21)
static inline void __cpuid(uint32 index, uint32 *eax, uint32 *ebx, uint32 *ecx, uint32 *edx)
{
    asm("cpuid"
        : "=a" (*eax), "=b" (*ebx), "=c" (*ecx), "=d" (*edx)
        : "0" (index));
}

static inline uint32 __ffs(uint32 word)
{
    asm("bsf %1,%0"
        : "=r" (word)
        : "rm" (word));
    return word;
}

static inline uint32 __fls(uint32 word)
{
    asm("bsr %1,%0"
        : "=r" (word)
        : "rm" (word));
    return word;
}

#endif