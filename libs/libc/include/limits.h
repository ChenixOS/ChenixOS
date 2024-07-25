#ifndef _LIMITS_H_
#define _LIMITS_H_

#define INT_MIN     0x80000000
#define INT_MAX     0x7FFFFFFF
#define UINT_MAX    0xFFFFFFFF

#define OPEN_MAX    32      /**< Number of open files */
#define NAME_MAX    64      /**< Chars in file name */
#define PATH_MAX    256     /**< Chars in a path */
#define ARG_MAX     1024    /**< Arguments and environment max length */
#define PIPE_BUF    512     /**< Bytes that can be written atomically */


#endif /* _LIMITS_H_ */