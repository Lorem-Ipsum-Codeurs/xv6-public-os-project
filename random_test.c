#include "types.h"
#include "stat.h"
#include "user.h"
#include "fcntl.h" // Include for scheduler policy defines

#define NUM_CHILDREN 5
// Base loop count for computation
#define BASE_LOOP 2000000

int main(int argc, char *argv[]) {
    // Set the scheduler policy to RANDOM for this test
    if (setschedpolicy(SCHED_RANDOM) < 0) {
        printf(1, "Error setting scheduler policy to RANDOM\n");
        exit();
    }
    printf(1, "Set scheduler to RANDOM (policy %d)\n", SCHED_RANDOM);

    printf(1, "Random Scheduler Test\n");
    printf(1, "---------------------\n");
    printf(1, "Creating %d processes with similar computation times\n", NUM_CHILDREN);
    printf(1, "Random scheduler should execute them in non-deterministic order\n");

    int pid, i;

    for(i = 0; i < NUM_CHILDREN; i++) {
        pid = fork();
        if(pid < 0) {
            printf(1, "Error: fork failed\n");
            exit();
        }
        if(pid == 0) {
            // Child process
            // Give them roughly similar, but not identical, work
            int loop_count = BASE_LOOP + (i * 100000); // Slight variation
            printf(1, "Child %d (PID %d) created. Computation loops: %d\n", i, getpid(), loop_count);
            printf(1, "Child %d (PID %d) starting computation...\n", i, getpid());

            // Perform computation
            volatile int j;
            for(j = 0; j < loop_count; j++) {
                // Busy loop
            }

            printf(1, "Child %d (PID %d) finished\n", i, getpid());
            exit();
        }
    }

    // Parent waits for all children
    printf(1, "Parent waiting for children...\n");
    printf(1, "The finish order should appear random and may vary between runs\n");
    for(i = 0; i < NUM_CHILDREN; i++) {
        wait();
    }

    printf(1, "All child processes have completed\n");
    printf(1, "Random scheduler test completed\n");

    exit();
}