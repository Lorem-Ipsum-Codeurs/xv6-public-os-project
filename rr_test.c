#include "types.h"
#include "stat.h"
#include "user.h"
#include "fcntl.h" // Include for scheduler policy defines

#define LOOPS 5
#define REPORT_INTERVAL 50000000ULL // Use unsigned long long

int
main(int argc, char *argv[])
{
  // Set the scheduler policy to RR for this test
  if (setschedpolicy(SCHED_RR) < 0) {
    printf(1, "Error setting scheduler policy to RR\n");
    exit();
  }
  printf(1, "Set scheduler to RR (policy %d)\n", SCHED_RR);

  printf(1, "Round Robin (RR) Scheduler Test\n");
  printf(1, "-------------------------------\n");
  printf(1, "Creating 3 long-running processes that report progress periodically\n");
  printf(1, "RR should interleave their execution, visible in progress reports\n");

  int pid, i;

  for(i = 0; i < 3; i++) {
    pid = fork();
    if(pid < 0) {
      printf(1, "Error: fork failed\n");
      exit();
    }
    if(pid == 0) {
      // Child process
      int child_id = i;
      int mypid = getpid();
      unsigned long long total_work = (unsigned long long)LOOPS * REPORT_INTERVAL;

      printf(1, "Child %d (PID %d) created. Total computation units: %d\n", child_id, mypid, total_work);
      printf(1, "Child %d (PID %d) starting long computation...\n", child_id, mypid);

      // Perform a longer computation, printing periodically
      volatile unsigned long long j;
      int k;

      for (k = 0; k < LOOPS; k++) {
          for(j = 0; j < REPORT_INTERVAL; j++) {
              // Busy loop
          }
          // Make progress report clearer
          printf(1, "  [RR Progress] Child %d (PID %d): Lap %d/%d\n", child_id, mypid, k + 1, LOOPS);
      }

      printf(1, "Child %d (PID %d) finished\n", child_id, mypid);
      exit();
    }
  }

  // Parent waits for all children
  printf(1, "Parent waiting for children...\n");
  printf(1, "Observe the interleaved progress reports from children.\n");
  printf(1, "This indicates Round Robin is working correctly.\n");
  for(i = 0; i < 3; i++) {
    wait();
  }

  printf(1, "All child processes have completed\n");
  printf(1, "RR scheduler test completed\n");

  exit();
}