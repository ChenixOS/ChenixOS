#pragma once

#include "types.h"

void kmemset(void* dest, int value, uint64 size);
void kmemcpy(void* dest, const void* src, uint64 size);
void kmemmove(void* dest, const void* src, uint64 size);

void kmemcpyb(void* dest, const void* src, uint64 count);
void kmemcpyd(void* dest, const void* src, uint64 count);
void kmemcpyq(void* dest, const void* src, uint64 count);

/**
 * This function does the same as kmemcpy, except that it does not cause an exception when reading/writing to an invalid address.
 * Returns true if copying was successful, false if the buffer was invalid
 **/
bool kmemcpy_usersafe(void* dest, const void* src, uint64 count);
/**
 * This function does the same as kmemset, except that it does not cause an exception when writing to an invalid address.
 * Returns true if writing was successful, false if the buffer was invalid
 **/
bool kmemset_usersafe(void* dest, int value, uint64 size);

bool kpathcpy_usersafe(char* dest, const char* src);

// 内存操作

static inline void writel(void *addr, uint32 val) {
    barrier();
    *(volatile uint32 *)addr = val;
}
static inline void writew(void *addr, uint16 val) {
    barrier();
    *(volatile uint16 *)addr = val;
}
static inline void writeb(void *addr, uint8 val) {
    barrier();
    *(volatile uint8 *)addr = val;
}
static inline uint64 readq(const void *addr) {
    uint64 val = *(volatile const uint64 *)addr;
    barrier();
    return val;
}
static inline uint32 readl(const void *addr) {
    uint32 val = *(volatile const uint32 *)addr;
    barrier();
    return val;
}
static inline uint16 readw(const void *addr) {
    uint16 val = *(volatile const uint16 *)addr;
    barrier();
    return val;
}
static inline uint8 readb(const void *addr) {
    uint8 val = *(volatile const uint8 *)addr;
    barrier();
    return val;
}