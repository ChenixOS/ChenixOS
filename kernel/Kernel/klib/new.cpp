#include "memory/KernelHeap.h"
#include "memory/kmalloc.h"

void* operator new(long unsigned int size)
{
    return kmalloc(size);
}
void* operator new[](long unsigned int size)
{
    return operator new(size);
}

void operator delete(void* block)
{
    kfree(block);
}
void operator delete(void* block, long unsigned int size)
{
    operator delete(block);
}

void operator delete[](void* block, long unsigned int size)
{
    operator delete(block);
}
void operator delete[](void* block)
{
    operator delete(block);
}