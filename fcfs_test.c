#include "types.h"
#include "stat.h"
#include "user.h"
#include "fcntl.h" // Include for scheduler policy defines

#define BASE_LOOP 1500000

int
main(int argc, char *argv[])
{
  // Set the scheduler policy to FCFS for this test
  if (setschedpolicy(SCHED_FCFS) < 0) {
    printf(1, "Error setting scheduler policy to FCFS\n");
    exit();
  }
  printf(1, "Set scheduler to FCFS (policy %d)\n", SCHED_FCFS);

  printf(1, "First-Come, First-Served (FCFS) Scheduler Test\n");
  printf(1, "---------------------------------------------\n");
  printf(1, "Creating processes in order: 0, 1, 2 (with different execution times)\n");
  printf(1, "FCFS should run them in creation order regardless of execution time\n");

  int pid, i;

  // Execution times for each process (different but shouldn't matter for FCFS)
  int computation_lengths[] = {2*BASE_LOOP, BASE_LOOP, 3*BASE_LOOP}; // Medium, Short, Long

  for(i = 0; i < 3; i++) {
    pid = fork();
    if(pid < 0) {
      printf(1, "Error: fork failed\n");
      exit();
    }
    if(pid == 0) {
      // Child process
      int len = computation_lengths[i];
      
      printf(1, "Child %d (PID %d) created. Computation length: %d\n", i, getpid(), len);
      printf(1, "Child %d (PID %d) starting computation...\n", i, getpid());

      // Perform computation
      volatile int j;
      for(j = 0; j < len; j++) {
        // Busy loop
      }

      printf(1, "Child %d (PID %d) finished\n", i, getpid());
      exit();
    }
  }

  // Parent waits for all children
  printf(1, "Parent waiting for children (expect finish order: 0, 1, 2)...\n");
  for(i = 0; i < 3; i++) {
    wait();
  }

  printf(1, "All child processes have completed\n");
  printf(1, "FCFS scheduler test completed\n");

  exit();
}
