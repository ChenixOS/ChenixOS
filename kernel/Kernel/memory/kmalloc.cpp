#include "kmalloc.h"
#include "KernelHeap.h"
#include "stdio.h"
#include "klib/memory.h"

#ifdef __cplusplus
extern "C" {
#endif

void* kmalloc(size_t size) {
    return KernelHeap::Allocate(size);
}

void kfree(void* addr) {
    KernelHeap::Free(addr);
}

void* krealloc(void *ptr, size_t size) {
    int bsize = KernelHeap::GetSize(ptr);
    if(bsize == size || bsize == 0 || size == 0)
        return ptr;
    
    void* result = kmalloc(size);
    kmemset(ptr, 0, size);
    kmemcpy(result, ptr, bsize);

    return result;
}


#ifdef __cplusplus
}
#endif