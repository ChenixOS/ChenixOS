#pragma once

#include "stdio.h"

#ifdef __cplusplus
extern "C" {
#endif

void* kmalloc(size_t size);

void kfree(void* addr);

void* krealloc(void *ptr, size_t size);

#ifdef __cplusplus
}
#endif