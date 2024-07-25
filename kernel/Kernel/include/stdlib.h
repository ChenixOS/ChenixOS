// 内核兼容stdlib.h
#ifndef _COMPACT_STDLIB_H_
#define _COMPACT_STDLIB_H_

#include "stdio.h"
#include "types.h"

// 引入内核管理函数
#include "memory/kmalloc.h"

// 内核分配函数
#define malloc(size) kmalloc(size)
#define free(addr) kfree(addr)
#define realloc(addr,size) krealloc(addr,size)

#endif