#include "types.h"
#include "stat.h"
#include "user.h"

int main() {
  int wtime, rtime;
  int pid = fork();

  if (pid == 0) {
    // Child process does some work
    printf(1, "Child process starting work...\n");
    for (volatile int i = 0; i < 1000000000; i++); // CPU work
    printf(1, "Child process completed\n");
    exit();
  } else if (pid > 0) {
    printf(1, "Parent waiting for child %d\n", pid);
    int cpid = waitx(&wtime, &rtime);
    printf(1, "Child %d finished with:\n", cpid);
    printf(1, "- Wait time: %d ticks\n", wtime);
    printf(1, "- Run time: %d ticks\n", rtime);
  } else {
    printf(1, "Fork failed\n");
  }
  
  exit();
}