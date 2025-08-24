#include "types.h"
#include "stat.h"
#include "user.h"
#include "fcntl.h" // Include for scheduler policy defines

int
main(int argc, char *argv[])
{
  // Set the scheduler policy to SJF for this test
  if (setschedpolicy(SCHED_SJF) < 0) {
    printf(1, "Error setting scheduler policy to SJF\n");
    exit();
  }
  printf(1, "Set scheduler to SJF (policy %d)\n", SCHED_SJF);

  printf(1, "Shortest Job First (SJF) Scheduler Test\n");
  printf(1, "----------------------------------------\n");
  printf(1, "Creating processes with varying computation times:\n");
  printf(1, "Process 0: Medium (2M), Process 1: Short (1M), Process 2: Long (5M)\n");

  int pid, i;

  // Fork multiple processes with different computation lengths
  // SJF should prioritize the shorter jobs first
  int computation_lengths[] = {2000000, 1000000, 5000000}; // Medium, Short, Long

  for(i = 0; i < 3; i++) {
    pid = fork();
    if(pid < 0) {
      printf(1, "Error: fork failed\n");
      exit();
    }
    if(pid == 0) {
      // Child process
      int len = computation_lengths[i];
      
      // KEY CHANGE: Set the estimated burst time to match computation length
      set_burst_estimate(len);
      
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
  printf(1, "Parent waiting for children (expect finish order: 1, 0, 2)...\n");
  for(i = 0; i < 3; i++) {
    wait();
  }

  printf(1, "All child processes have completed\n");
  printf(1, "Observe the finish order - shorter jobs should finish first.\n");
  printf(1, "SJF scheduler test completed\n");

  exit();
}
