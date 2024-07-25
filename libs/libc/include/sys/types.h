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

#define makedev(maj, min)  ((((maj) & 0xFF) << 8) | ((min) & 0xFF))
#define major(dev) (((dev) >> 8) & 0xFF)
#define minor(dev) ((dev) & 0xFF)

#endif /* _SYS_TYPES_H_ */