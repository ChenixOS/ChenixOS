// 内核兼容string.h
#ifndef _COMPACT_STRING_H_
#define _COMPACT_STRING_H_

#include "types.h"

#include "klib/memory.h"

#define memset kmemset
#define memcpy kmemcpy
#define memmove kmemmove

#include "klib/string.h"

#define strcmp kstrcmp
#define strncmp kstrncmp
#define strcpy kstrcpy
#define strlen kstrlen

#endif