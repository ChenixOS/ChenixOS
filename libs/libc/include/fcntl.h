#ifndef _FCNTL_H_
#define _FCNTL_H_

/**
 * Main access modes.
 * @{
 */
#define O_ACCMODE       03       /**< Access mode mask */
#define O_RDONLY        00       /**< Read access */
#define O_WRONLY        01       /**< Write access */
#define O_RDWR          02       /**< Read/write access */
#define O_CREAT         0100     /**< Create if not exists (not fcntl) */
#define O_TRUNC         01000    /**< Truncate if exists (not fcntl) */
#define O_APPEND        02000    /**< Append if exists */
#define O_NONBLOCK      04000    /**< Open in non blocking mode (read/write) */
#define O_CLOEXEC       010000   /**< Close on exec */
#define O_NOCTTY        020000   /**< Do not allocate controlling terminal */
/** @} */

/**
 * Flags to be set with
 *   fcntl(fd, F_SETFD, ...)
 */
#define FD_CLOEXEC      1

#endif /* _FCNTL_H_ */