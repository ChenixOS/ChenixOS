#ifndef _STDDEF_H_
#define _STDDEF_H_


#include <sys/types.h>
#include <stdint.h>


/** Null pointer constant. */
#if defined(__cplusplus)
#define NULL 0
#else
#define NULL ((void *)0)
#endif

/**
 * Integer constant expression of type size_t, the value of which is the
 * offset in bytes to the structure member (member-designator), from the
 * beginning of its structure (type).
 */
#define offsetof(type, memb) ((size_t)((char *)&((type *)0)->memb - (char *)0))

typedef long ptrdiff_t;

#endif /* _STDDEF_H_ */
