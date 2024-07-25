#pragma once

#include "types.h"
#include "MemoryManager.h"

namespace KernelHeap {

    /**
     * Allocates [size] bytes on the kernel heap. !Can only be used while Interrupts are enabled!
     * The advantage over MemoryManager::AllocatePages is that the physical memory does not have to be contiguous.
     **/
    void* Allocate(uint64 size);

    /**
     * Frees memory allocated with Allocate
     **/
    void Free(void* block);

    // 获取内存块大小
    uint64 GetSize(void* block);

    inline void* AllocPages(uint32 count = 1) {
        return MemoryManager::PhysToKernelPtr(MemoryManager::AllocatePages(count));
    }

    inline void FreePages(void* ptr,uint32 count = 1) {
        MemoryManager::FreePages(MemoryManager::KernelToPhysPtr(ptr),count);
    }
}