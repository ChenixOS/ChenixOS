#pragma once

#define syscall_open 1
#define syscall_close 2  
#define syscall_read 3 
#define syscall_write 4
#define syscall_mount 5 
#define syscall_mount_dev 6 
#define syscall_change_owner 7 
#define syscall_change_perm 8 
#define syscall_copyfd 9 
#define syscall_reopenfd 10 
#define syscall_seek 11 
#define syscall_create_symlink 12 
#define syscall_create_hardlink 13 
#define syscall_unmount 14 
#define syscall_stat 15 
#define syscall_dev_cmd 16 
#define syscall_list 17
#define syscall_taskctl 18

#define syscall_wait 18 
#define syscall_exit 19 
#define syscall_fork 20 
#define syscall_exec 21 
#define syscall_print 22 
#define syscall_statl 23 
#define syscall_cd 24 
#define syscall_pwd 25 
#define syscall_abort 26 


#define syscall_create_file 28 
#define syscall_create_folder 29 
#define syscall_create_dev 30 
#define syscall_delete 31 
#define syscall_pipe 32

#define syscall_alloc 100 
#define syscall_free 101 
#define syscall_mm_query 102


#define syscall_getuid 201 
#define syscall_getgid 202
#define syscall_getpid 203
#define syscall_getppid 204
#define syscall_setuid 205
#define syscall_handle_kill 206 
#define syscall_finish_kill 207
#define syscall_whoami 208 
#define syscall_gettime 209
#define syscall_setgid 210

#define syscall_socket 301
#define syscall_socket_bind 302
#define syscall_socket_listen 303
#define syscall_socket_accept 304
#define syscall_socket_connect 305
#define syscall_socket_send 306
#define syscall_socket_recv 307
#define syscall_socket_setsockopt 308
#define syscall_socket_getsockopt 309
#define syscall_socket_poll 310
#define syscall_socket_select 311
#define syscall_check_is_socket 312

#define syscall_socket_sendto 313
#define syscall_socket_recvfrom 314

#define syscall_gettid 500
#define syscall_setfs 501 
#define syscall_thread_create 502
#define syscall_thread_join 503
#define syscall_thread_tryjoin 504 
#define syscall_thread_kill 505
#define syscall_thread_detach 506
#define syscall_thread_setcore 507
#define syscall_thread_exit 508