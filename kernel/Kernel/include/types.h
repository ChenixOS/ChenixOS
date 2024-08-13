#ifndef _SYS_TYPES_H_
#define _SYS_TYPES_H_

#include <stdint.h>

typedef unsigned int    size_t;
typedef int             ssize_t;

typedef int             pid_t;
typedef uint16_t        dev_t;
typedef uint16_t        ino_t;
typedef uint16_t        mode_t;
typedef unsigned int    nlink_t;
typedef unsigned int    uid_t;
typedef unsigned int    gid_t;
typedef unsigned int    id_t;
typedef long int        off_t;
typedef long int        blksize_t;
typedef long int        blkcnt_t;

union u64_u32_u {
    struct { uint32 lo, hi; };
    uint64 val;
};

// Definition for common 16bit segment/offset pointers.
struct segoff_s {
    union {
        struct {
            uint16 offset;
            uint16 seg;
        };
        uint32 segoff;
    };
};

#define makedev(maj, min)  ((((maj) & 0xFF) << 8) | ((min) & 0xFF))
#define major(dev) (((dev) >> 8) & 0xFF)
#define minor(dev) ((dev) & 0xFF)

#define ARRAY_SIZE(a) (sizeof(a) / sizeof(a[0]))
#define FIELD_SIZEOF(t, f) (sizeof(((t*)0)->f))

#define barrier() __asm__ __volatile__("": : :"memory")

#define __noreturn __attribute__((noreturn))
#define noinline __attribute__((noinline))
#define __always_inline inline __attribute__((always_inline))

#define __stringify_1(x)        #x
#define __stringify(x)          __stringify_1(x)


#define ALIGN(x,a)              __ALIGN_MASK(x,(a)-1)
#define __ALIGN_MASK(x,mask)    (((x)+(mask))&~(mask))
#define ALIGN_DOWN(x,a)         ((x) & ~((a)-1))


#endif /* _SYS_TYPES_H_ */