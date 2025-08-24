// System call numbers
#define SYS_fork    1
#define SYS_exit    2
#define SYS_wait    3
#define SYS_pipe    4
#define SYS_read    5
#define SYS_kill    6
#define SYS_exec    7
#define SYS_fstat   8
#define SYS_chdir   9
#define SYS_dup    10
#define SYS_getpid 11
#define SYS_sbrk   12
#define SYS_sleep  13
#define SYS_uptime 14
#define SYS_open   15
#define SYS_write  16
#define SYS_mknod  17
#define SYS_unlink 18
#define SYS_link   19
#define SYS_mkdir  20
#define SYS_close  21
#define SYS_find_digit_root 22 // New system call to find the digit root of a number
#define SYS_copy_file 23 // New system call to copy a file
#define SYS_get_uncle_count 24 // New system call to get the uncle count
#define SYS_get_process_lifetime 25 
#define SYS_getnumsyscalls 26
#define SYS_getnumsyscallsgood 27
#define SYS_waitx 28
#define SYS_set_priority 29
#define SYS_ps      30
#define SYS_test_rr 31  // New system call for testing RR
#define SYS_setschedpolicy 32 // New system call for setting scheduling policy
#define SYS_set_burst_estimate 33
#define SYS_yield     34
