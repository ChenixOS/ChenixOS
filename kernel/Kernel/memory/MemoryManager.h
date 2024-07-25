#pragma once

#include "types.h"
#include "KernelHeader.h"

#define NUM_PAGES(size) (((size) + 4095) / 4096)

#define __kernelptr
#define __userptr
#define __phys

namespace MemoryManager {

    bool Init(KernelHeader* header);
    void InitCore();

    void DisableChacheOnLargePage(void* virt);
    void EnableWriteCombineOnLargePage(void* virt);

    /**
     * Does the same as AllocatePages, but can be called before the scheduler is initialized
     **/
    __phys void* EarlyAllocatePages(uint64 numPages);
    /**
     * Does the same as FreePages, but can be called before the scheduler is initialized
     **/
    void EarlyFreePages(__phys void* pages, uint64 numPages);

    /**
     * Allocate physically continuous pages.
     * Can only be called from a thread.
     **/
    __phys void* AllocatePages(uint64 numPages = 1);
    /**
     * Free pages Allocated with AllocatePages.
     * Can only be called from a thread.
     */
    void FreePages(__phys void* pages, uint64 numPages = 1);

    /**
     * Ejects the given memory page from the CPU translation chaches.
     * Should be called after changing any page table entries.
     **/
    void InvalidatePage(__phys void* page);

    /**
     * Convert the given physical address to a pointer that can be accessed by the kernel
     **/
    void* PhysToKernelPtr(const void* ptr);
    /**
     * Convert the given Kernel pointer to the physical address it represents
     **/
    void* KernelToPhysPtr(const void* ptr);

    /**
     * Create a new Paging structure to be used by a user process
     **/
    uint64 CreateProcessMap();
    /**
     * Clone the User Memory space of the current process
     **/
    uint64 ForkProcessMap();
    /**
     * Clean up the User Memory space associated with the given pml4Entry
     **/
    void FreeProcessMap(uint64 pml4Entry);

    /**
     * Switch to a given User Memory Space
     **/
    void SwitchProcessMap(uint64 pml4Entry);

    /**
     * Map a physical page to be accessible by the kernel only
     **/
    void MapKernelPage(void* phys, void* virt);
    /**
     * Map a physical page into the given User Memory Space
     **/
    void MapProcessPage(uint64 pml4Entry, void* phys, void* virt, bool invalidate = true);
    /**
     * Map any available physical page to the given virtual page. If the virtual page is already mapped to any physical page, do nothing
     **/
    void* MapProcessPage(uint64 pml4Entry, void* virt, bool invalidate);
    void MapProcessPage(void* virt);
    /**
     * Remove a page from the given User Memory Space
     **/
    void UnmapProcessPage(uint64 pml4Entry, void* virt);
    void UnmapProcessPage(void* virt);

    /**
     * Return a kernel-usable pointer to the physical page mapped to the given virtual address. If it is not mapped, return nullptr
     **/
    void* UserToKernelPtr(const void* ptr);

    /**
     * Check if the given pointer is a userspace address.
     * Does *not* check whether the given pointer is valid.
     **/
    bool IsUserPtr(const void* ptr);

    /* Auto let user or kernel ptr convert to physical ptr */
    void* UserOrKernelToPhysPtr(void* addr);

    /*inline uint32 KernelTo32PhysPtr(__kernelptr void* ptr) {
        return (uint32)(((uint64)KernelToPhysPtr(ptr))& 0xFFFFFFFF);
    }*/

    inline void SplitKernelToPhysPtr(__kernelptr void* ptr,uint32* high,uint32* low) {
        uint64 phys = (uint64)KernelToPhysPtr(ptr);
        *high = (uint32)(phys >> 32); // 获取高32位
        // *high = 0;
        *low = (uint32)(phys & 0xFFFFFFFF); // 获取低32位
    }
}