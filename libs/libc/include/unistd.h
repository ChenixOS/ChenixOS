#ifndef _LIBC_UNISTD_H_

#define _LIBC_UNISTD_H_

#include <fcntl.h>
#include <sys/types.h>

#ifdef __cplusplus
extern "C" {
#endif

extern char **environ;  /**< Pointer to the environment strings */

#define STDIN_FILENO        0
#define STDOUT_FILENO       1
#define STDERR_FILENO       2

/* Values for the 'whence' argument to lseek */
#define SEEK_SET            0
#define SEEK_CUR            1
#define SEEK_END            2

/* Values for the second argument to access.
 * These may be OR'd together.  */
#define F_OK                0       /* Test for existence */
#define X_OK                1       /* Test for execute permission */
#define W_OK                2       /* Test for write permission */
#define R_OK                4       /* Test for read permission */

int open(const char* path, unsigned int mode,unsigned int flags);
int close(int fd);
int read(int fd, void* buffer, unsigned int bufferSize);
int write(int fd, const void* buffer, unsigned int bufferSize);
int lseek(int fd, off_t offset, int whence);
int chdir(const char* path);
int getpwd(char* pathBuffer);
int dup2(int oldfd,int newfd);
int dup(int oldfd);
int tcsetpgrp(int fd,pid_t pgrpid);

pid_t fork();

void execve(const char* path, int argc, const char* const* argv,const char** environ);
pid_t getpid();
pid_t getppid();

uid_t getuid();
uid_t getgid();

#ifdef __cplusplus
}
#endif


#endif