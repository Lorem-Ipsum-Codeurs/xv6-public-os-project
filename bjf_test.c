#include "types.h"
#include "stat.h"
#include "user.h"
#include "fcntl.h" // Include for scheduler policy defines

#define LOOP_MEDIUM 2000000 

int main(int argc, char *argv[]) {
  // Set the scheduler policy to BJF for this test
  if (setschedpolicy(SCHED_BJF) < 0) {
    printf(1, "Error setting scheduler policy to BJF\n");
    exit();
  }
  printf(1, "Set scheduler to BJF (policy %d)\n", SCHED_BJF);

  printf(1, "Best Job First (BJF) Scheduler Test\n");
  printf(1, "------------------------------------\n");
  printf(1, "Creating processes with different priorities and computation times:\n");
  printf(1, "Process 0: High Priority (10), Medium Length\n");
  printf(1, "Process 1: Low Priority (90), Medium Length\n"); 
  printf(1, "Process 2: Medium Priority (50), Medium Length\n");
  printf(1, "(Lower priority number means higher priority)\n");

  int pid, i;
  // Priorities: High (10), Low (90), Medium (50)
  int priorities[] = {10, 90, 50};
  // Use the same computation length to isolate priority effect
  int computation_lengths[] = {LOOP_MEDIUM, LOOP_MEDIUM, LOOP_MEDIUM};

  for(i = 0; i < 3; i++) {
    pid = fork();
    if(pid < 0) {
      printf(1, "Error: fork failed\n");
      exit();
    }
    if(pid == 0) {
      // Child process
      int my_priority = priorities[i];
      int len = computation_lengths[i];

      // Set priority
      if (set_priority(my_priority) < 0) {
         printf(1, "Child %d (PID %d): Warning - set_priority failed.\n", i, getpid());
      } else {
         printf(1, "Child %d (PID %d) priority set to %d\n", i, getpid(), my_priority);
      }

      printf(1, "Child %d (PID %d) created. Computation length: %d\n", i, getpid(), len);
      printf(1, "Child %d (PID %d) starting computation...\n", i, getpid());

      // Perform computation
      volatile int j;
      for(j = 0; j < len; j++) {
        // Busy loop
        
        // Yield periodically to give scheduler a chance to run other processes
        if(j % 200000 == 0) {
          yield();
        }
      }

      printf(1, "Child %d (PID %d) finished\n", i, getpid());
      exit();
    }
  }

  // Parent process: Give children time to set up
  sleep(10); // Sleep to ensure all children run set_priority
  
  // Parent waits for all children
  printf(1, "Parent waiting for children (expect finish order based on priority: 0, 2, 1)...\n");
  for(i = 0; i < 3; i++) {
    wait();
  }

  printf(1, "All child processes have completed\n");
  printf(1, "Observe the finish order based on BJF criteria (priority, then creation time).\n");
  printf(1, "BJF scheduler test completed\n");

  exit();
}
