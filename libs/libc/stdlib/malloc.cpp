#include "../internal/syscall.h"
#include "../internal/FreeList.h"
#include <stdlib.h>
#include <string.h>

int64 alloc_pages(void* addr, uint64 numPages) {
    return syscall_invoke(syscall_alloc, (uint64)addr, numPages,0,0);
}
int64 free_pages(void* addr, uint64 numPages) {
    return syscall_invoke(syscall_free, (uint64)addr, (uint64)numPages,0,0);
}

static FreeList g_FreeList;
static uint64 g_HeapPos = 0xFF000000;

static void ReserveNew(uint64 size) {
    size = (size + 4095) / 4096;
    alloc_pages((void*)g_HeapPos, size);
    g_FreeList.MarkFree((void*)g_HeapPos, size * 4096);
    g_HeapPos += size * 4096;
}

extern "C"
void* malloc(size_t size) {
    size = (size + sizeof(uint64) * 2 + 63) / 64 * 64;

    void* g = g_FreeList.FindFree(size);
    if(g == nullptr) {
        ReserveNew(size);
        g = g_FreeList.FindFree(size);
    }
    g_FreeList.MarkUsed(g, size);
    memset(g, 0, size);

    *(uint64*)g = size;
    return (uint64*)g + 2;
}

extern "C"
void free(void* block) {
    if(block == nullptr)
        return;

    uint64* b = (uint64*)block - 2;
    uint64 size = *b;

    g_FreeList.MarkFree(b, size);
}

extern "C"
void *realloc(void *ptr, size_t size)
{
    if(ptr == nullptr) {
        return malloc(size);
    }
    uint64* b = (uint64*)ptr - 2;
    uint64 srcSize = *b;
    if(size == srcSize) {
        return ptr;
    } else if(size > srcSize) {
        // 扩大
        // TODO: 动态扩大，代替现有的低效方式
        void* res = malloc(size);
        memcpy(res,ptr,srcSize);
        return res;
    } else {
        // 缩小
        // TODO: 动态扩大，代替现有的低效方式
        void* res = malloc(size);
        memcpy(res,ptr,size);
        return res;
    }
}
