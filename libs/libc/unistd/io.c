/*
    Copyright [yyyy] [name of copyright owner]

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

        http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License. 
*/

#include "../internal/syscall.h"
#include <fcntl.h>
#include <sys/types.h>

int open(const char* path, unsigned int mode,unsigned int flags) {
    return syscall_invoke(syscall_open, (uint64_t)path, mode, flags, 0);
}

int close(int fd) {
    return syscall_invoke(syscall_close, fd, 0, 0, 0);
}

int read(int fd, void* buffer, unsigned int bufferSize) {
    return syscall_invoke(syscall_read, (uint64_t)fd, (uint64_t)buffer, bufferSize,0);
}

int write(int fd, const void* buffer, unsigned int bufferSize) {
    return syscall_invoke(syscall_write, (uint64_t)fd, (uint64_t)buffer, (uint64_t)bufferSize,0);
}

int lseek(int fd, off_t offset, int whence)
{
    return syscall_invoke(syscall_seek, fd, whence, offset, 0);
}

int chdir(const char* path) {
    return syscall_invoke(syscall_cd, (uint64_t)path,0,0,0);
}

int getpwd(char* pathBuffer) {
    return syscall_invoke(syscall_pwd, (uint64_t)pathBuffer,0,0,0);
}

int dup2(int oldfd,int newfd) {
    return syscall_invoke(syscall_copyfd,oldfd,newfd,0,0);
}

int dup(int oldfd) {
    int newfd = open("/dev/zero",O_RDONLY,0600);
    return dup2(oldfd,newfd);
}

int tcsetpgrp(int fd,pid_t pgrpid) {
    syscall_invoke(syscall_dev_cmd,fd,1,pgrpid,0);
}