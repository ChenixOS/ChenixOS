#include "socket.h"
#include "syscalls/SyscallDefine.h"

int socket_create(int domain, int type, int protocol) {

}

SYSCALL_DEFINE3(syscall_socket,int domain, int type, int protocol) {
    return socket_create(domain,type,protocol);
}