#include "syscall.h"
#include "traps.h"

#define SYSCALL(name) \
  .globl name; \
  name: \
    movl $SYS_ ## name, %eax; \
    int $T_SYSCALL; \
    ret

SYSCALL(fork)
SYSCALL(exit)
SYSCALL(wait)
SYSCALL(pipe)
SYSCALL(read)
SYSCALL(write)
SYSCALL(close)
SYSCALL(kill)
SYSCALL(exec)
SYSCALL(open)
SYSCALL(mknod)
SYSCALL(unlink)
SYSCALL(fstat)
SYSCALL(link)
SYSCALL(mkdir)
SYSCALL(chdir)
SYSCALL(dup)
SYSCALL(getpid)
SYSCALL(sbrk)
SYSCALL(sleep)
SYSCALL(uptime)
SYSCALL(find_digit_root) // returns the digit root of a number for using in random_seed
SYSCALL(copy_file); // function declaration for the copy file system call as a load for child process
SYSCALL(get_uncle_count) // returns the number of uncles for a given process
SYSCALL(get_process_lifetime) // returns the lifetime of a process
SYSCALL(getnumsyscalls) // returns the number of system calls made by a process
SYSCALL(getnumsyscallsgood) // returns the number of successful system calls made by a process
SYSCALL(waitx) // Add waitx syscall stub returns the exit status of a process and wait and execution time
SYSCALL(setschedpolicy) // Add new syscall entry for setschedpolicy
SYSCALL(set_priority) // sets the priority of a process in the scheduler
SYSCALL(ps)  // prints the process table
SYSCALL(test_rr) // test round robin scheduler
SYSCALL(set_burst_estimate) // sets the burst estimate of a process in the scheduler
SYSCALL(yield) // yield the CPU to another process



