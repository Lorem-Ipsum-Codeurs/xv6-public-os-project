#include "types.h"
#include "x86.h"
#include "defs.h"
#include "date.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "proc.h"
#include "spinlock.h"  // Add this line
#include "fcntl.h" // Include for scheduler policy defines

extern struct {
  struct spinlock lock;
  struct proc proc[NPROC];
} ptable;

// Also include for tickslock
extern struct spinlock tickslock;
extern uint ticks;

// Extern the global scheduler variable from proc.c
extern int current_scheduler;

int
sys_fork(void)
{
  return fork();
}

int
sys_exit(void)
{
  exit();
  return 0;  // not reached
}

int
sys_wait(void)
{
  return wait();
}

int
sys_kill(void)
{
  int pid;

  if(argint(0, &pid) < 0)
    return -1;
  return kill(pid);
}

int
sys_getpid(void)
{
  return myproc()->pid;
}

int
sys_sbrk(void)
{
  int addr;
  int n;

  if(argint(0, &n) < 0)
    return -1;
  addr = myproc()->sz;
  if(growproc(n) < 0)
    return -1;
  return addr;
}

int
sys_sleep(void)
{
  int n;
  uint ticks0;

  if(argint(0, &n) < 0)
    return -1;
  acquire(&tickslock);
  ticks0 = ticks;
  while(ticks - ticks0 < n){
    if(myproc()->killed){
      release(&tickslock);
      return -1;
    }
    sleep(&ticks, &tickslock);
  }
  release(&tickslock);
  return 0;
}

// return how many clock tick interrupts have occurred
// since start.
int
sys_uptime(void)
{
  uint xticks;

  acquire(&tickslock);
  xticks = ticks;
  release(&tickslock);
  return xticks;
}

// New system call to find the digit root of a number
// The digit root is the single digit obtained by an iterative process of summing digits,
// until a single digit is obtained. For example, the digit root of 38 is 2 (3 + 8 = 11, 1 + 1 = 2).
// This function takes an integer as input and returns its digit root.

int sys_find_digit_root(void) {
  int num;
  if (argint(0, &num) < 0)
    return -1;

  int root = 0;
  while (num > 0 || root > 9) {
    if (num == 0) {
      num = root;
      root = 0;
    }
    root += num % 10;
    num /= 10;
  }
  return root;
}


int
sys_get_uncle_count(void)
{
  int pid;
  
  if(argint(0, &pid) < 0)
    return -1;
  
  return get_uncle_count(pid);
}



int
sys_get_process_lifetime(void)
{
  int pid;
  struct proc *p;
  uint lifetime = 0;

  if(argint(0, &pid) < 0)
    return -1;

  acquire(&ptable.lock);

  for(p = ptable.proc; p < &ptable.proc[NPROC]; p++){
    if(p->state != UNUSED && p->pid == pid){
      acquire(&tickslock);
      lifetime = ticks - p->createtime;
      release(&tickslock);
      release(&ptable.lock);
      return lifetime;
    }
  }

  release(&ptable.lock);
  return -1; // PID not found
}

int
sys_getnumsyscalls(void)
{
  return myproc()->syscall_count;
}

int
sys_getnumsyscallsgood(void)
{
  return myproc()->good_syscall_count;
}

int
sys_waitx(void)
{
  int *wtime, *rtime;
  
  if(argptr(0, (char**)&wtime, sizeof(int)) < 0 || 
     argptr(1, (char**)&rtime, sizeof(int)) < 0)
    return -1;
  
  return waitx(wtime, rtime);
}

int
sys_set_priority(void)
{
  int new_priority;
  
  if(argint(0, &new_priority) < 0)
    return -1;
  
  struct proc *p = myproc();
  p->priority = new_priority;
  
  return 0;
}

// System call to display process information
int
sys_ps(void)
{
  struct proc *p;
  static char *states[] = {
    [UNUSED]    "UNUSED  ",
    [EMBRYO]    "EMBRYO  ",
    [SLEEPING]  "SLEEPING",
    [RUNNABLE]  "RUNNABLE",
    [RUNNING]   "RUNNING ",
    [ZOMBIE]    "ZOMBIE  "
  };
  
  cprintf("PID\tSTATE\t\tPRIORITY\tRUNTIME\tBURST\tNAME\n");
  cprintf("--------------------------------------------------\n");
  
  acquire(&ptable.lock);
  for(p = ptable.proc; p < &ptable.proc[NPROC]; p++){
    if(p->state == UNUSED)
      continue;
    
    cprintf("%d\t%s\t%d\t\t%d\t%d\t%s\n", 
      p->pid, 
      states[p->state], 
      p->priority, 
      p->runtime,
      p->estimated_burst,
      p->name);
  }
  release(&ptable.lock);
  
  return 0;
}

int
sys_test_rr(void)
{
  int n, i;
  int pid;
  int start_time, elapsed;
  
  // Get number of test processes to create (default to 3 if not specified)
  if(argint(0, &n) < 0)
    n = 3;
    
  // Ensure reasonable bounds
  if(n < 1) n = 1;
  if(n > 8) n = 8;
  
  cprintf("\n=== ROUND ROBIN SCHEDULING TEST ===\n");
  cprintf("Creating %d CPU-intensive processes\n", n);
  cprintf("Each process will report its progress periodically\n");
  cprintf("You should observe interleaved execution if RR is working\n\n");
  
  // Record starting time
  acquire(&tickslock);
  start_time = ticks;
  release(&tickslock);
  
  // Fork test processes
  for(i = 0; i < n; i++) {
    pid = fork();
    
    if(pid < 0) {
      cprintf("Fork failed in test_rr system call\n");
      return -1;
    }
    
    if(pid == 0) {
      // This is a child process - do CPU intensive work
      int j, k;
      int iterations = 10;
      int child_id = i + 1;
      
      for(j = 0; j < iterations; j++) {
        // CPU-intensive work
        int result = 0;
        for(k = 0; k < 100000000; k++) {
          result += k;
          if (k % 10000000 == 0) {
            // Force a brief yield to let output occur smoothly
            yield();
          }
        }
        
        // Report progress
        cprintf("Process %d: completed iteration %d/%d\n", 
                child_id, j+1, iterations);
      }
      
      cprintf("Process %d: FINISHED\n", child_id);
      exit();
    }
  }
  
  // Parent process waits for all children
  for(i = 0; i < n; i++) {
    wait();
  }
  
  // Calculate elapsed time
  acquire(&tickslock);
  elapsed = ticks - start_time;
  release(&tickslock);
  
  cprintf("\n=== TEST COMPLETE ===\n");
  cprintf("All processes completed in %d timer ticks\n", elapsed);
  cprintf("If you observed interleaved outputs from different processes,\n");
  cprintf("then Round Robin scheduling is working correctly.\n");
  
  return 0;
}

int
sys_setschedpolicy(void)
{
  int policy;

  if(argint(0, &policy) < 0)
    return -1;

  // Validate the policy value
  if (policy < SCHED_FCFS || policy > SCHED_RR) {
     return -1; // Invalid policy number
  }

  // Acquire lock to safely modify the global variable
  acquire(&ptable.lock);
  current_scheduler = policy;
  release(&ptable.lock);

  cprintf("Scheduler policy changed to %d\n", policy); // Optional: confirmation message

  return 0; // Success
}

int
sys_set_burst_estimate(void)
{
  int burst;
  
  if(argint(0, &burst) < 0)
    return -1;
    
  // Don't allow negative values
  if(burst < 0)
    return -1;
    
  struct proc *p = myproc();
  p->estimated_burst = burst;
  
  return 0;
}

// Add this function
int
sys_yield(void)
{
  yield();
  return 0;
}
