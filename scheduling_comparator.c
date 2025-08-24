#include "types.h"
#include "stat.h"
#include "user.h"
#include "fcntl.h" // Include for scheduler policy defines

// Structure to hold results for one scheduler
struct SchedResult {
    int policy;
    char *name;
    int rtime;         // Runtime
    int wtime;         // Wait time
    int syscalls;      // Total syscalls
    int goodsyscalls;  // Successful syscalls
    int lifetime;      // Process lifetime
    int uncles;        // Uncle count
};

// Test a specific workload pattern (optional)
void create_test_workload(char *filename) {
    // Create a test file with some content
    copy_file("README", filename);
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf(2, "Usage: scheduling_comparator <program_to_run> [args...]\n");
        exit();
    }

    char *program_name = argv[1];
    char **program_args = argv + 1; // Pass remaining args to the target program
    
    // Create or use a test workload (optional)
    char test_file[32] = "test_workload";
    int use_workload = 0;
    
    if (use_workload) {
        create_test_workload(test_file);
    }

    // Define the schedulers to test
    struct SchedResult results[] = {
        {SCHED_FCFS,   "FCFS  ", -1, -1, 0, 0, 0, 0},
        {SCHED_SJF,    "SJF   ", -1, -1, 0, 0, 0, 0},
        {SCHED_BJF,    "BJF   ", -1, -1, 0, 0, 0, 0},
        {SCHED_RANDOM, "RANDOM", -1, -1, 0, 0, 0, 0},
        {SCHED_RR,     "RR    ", -1, -1, 0, 0, 0, 0}
    };
    int num_schedulers = sizeof(results) / sizeof(results[0]);
    // int original_policy = 0; // Store current policy if available

    // Use a random seed based on the digital root if needed
    int seed = find_digit_root(uptime());
    printf(1, "--- Scheduling Policy Comparison for '%s' (Seed: %d) ---\n", program_name, seed);

    for (int i = 0; i < num_schedulers; i++) {
        printf(1, "\nTesting Scheduler: %s (%d)\n", results[i].name, results[i].policy);

        // Set the scheduling policy
        if (setschedpolicy(results[i].policy) < 0) {
            printf(2, "Error setting scheduler policy %s\n", results[i].name);
            continue; 
        } else {
            printf(1, "Policy set to %s\n", results[i].name);
        }

        // For BJF scheduler, set some priority and burst estimate
        if (results[i].policy == SCHED_BJF) {
            set_priority(10); // Medium priority
        }
        
        // For SJF, set an initial burst estimate
        if (results[i].policy == SCHED_SJF) {
            set_burst_estimate(50);
        }

        int start_syscalls = getnumsyscalls();
        int start_goodsyscalls = getnumsyscallsgood();
        
        int pid = fork();

        if (pid < 0) {
            printf(2, "Fork failed\n");
            continue;
        } else if (pid == 0) {
            // Child process
            
            // Child can record its own metrics before exec
            // (these will be lost when exec runs, so only useful for debugging)
            int my_lifetime = get_process_lifetime(getpid());
            int my_uncles = get_uncle_count(getpid());
            printf(1, "Child process %d before exec: Lifetime=%d, Uncles=%d\n", 
                  getpid(), my_lifetime, my_uncles);
            
            // Execute the target program
            exec(program_name, program_args);
            // If exec returns, it failed
            printf(2, "Exec '%s' failed\n", program_name);
            exit(); // Child exits on failure
        } else {
            // Parent process
            
            // Get measurements while child is running
            sleep(20); // Small delay to ensure child process has started
            
            results[i].lifetime = get_process_lifetime(pid);
            results[i].uncles = get_uncle_count(pid);
            printf(1, "Child %d metrics during execution: Life: %d, Uncles: %d\n", 
                  pid, results[i].lifetime, results[i].uncles);
                  
            // Wait for the child and collect stats
            int wtime, rtime;
            int status = waitx(&wtime, &rtime);
            
            if (status < 0) {
                printf(2, "Waitx failed for policy %s\n", results[i].name);
            } else {
                results[i].wtime = wtime;
                results[i].rtime = rtime;
                results[i].syscalls = getnumsyscalls() - start_syscalls;
                results[i].goodsyscalls = getnumsyscallsgood() - start_goodsyscalls;
                
                printf(1, "PID %d finished. Status: %d, Wait: %d, Run: %d, Life: %d, Syscalls: %d/%d, Uncles: %d\n",
                      pid, status, wtime, rtime, results[i].lifetime, 
                      results[i].goodsyscalls, results[i].syscalls, results[i].uncles);
            }
        }
    }

    // Show process table (optional)
    printf(1, "\nProcess table at completion:\n");
    ps();

    // Print summary table
    printf(1, "\n--- Comparison Summary ---\n");
    printf(1, "Scheduler | Wait Time | Run Time | Total | Syscalls | Lifetime | Uncles\n");
    printf(1, "----------|-----------|----------|-------|----------|----------|-------\n");

    int best_rtime = -1;
    int best_total_time = -1;
    int best_rtime_idx = -1;
    int best_total_time_idx = -1;

    for (int i = 0; i < num_schedulers; i++) {
        if (results[i].rtime != -1) { // Check if the test ran successfully
            int total_time = results[i].rtime + results[i].wtime;
            
            printf(1, "%s | %d       | %d      | %d    | %d/%d    | %d       | %d\n",
                   results[i].name, results[i].wtime, results[i].rtime, total_time,
                   results[i].goodsyscalls, results[i].syscalls, 
                   results[i].lifetime, results[i].uncles);

            // Track best runtime
            if (best_rtime_idx == -1 || results[i].rtime < best_rtime) {
                best_rtime = results[i].rtime;
                best_rtime_idx = i;
            }
            // Track best total time
            if (best_total_time_idx == -1 || total_time < best_total_time) {
                best_total_time = total_time;
                best_total_time_idx = i;
            }
        } else {
            printf(1, "%s | ---       | ---      | ---   | ---      | ---      | ---\n", 
                   results[i].name);
        }
    }

    // Suggest best scheduler
    printf(1, "\n--- Suggestion ---\n");
    if (best_rtime_idx != -1) {
        printf(1, "Best scheduler based on Run Time: %s (Runtime: %d)\n",
               results[best_rtime_idx].name, results[best_rtime_idx].rtime);
    } else {
        printf(1, "Could not determine best scheduler based on runtime.\n");
    }
    
    if (best_total_time_idx != -1) {
        printf(1, "Best scheduler based on Total Time: %s (Total Time: %d)\n",
               results[best_total_time_idx].name, best_total_time);
    } else {
        printf(1, "Could not determine best scheduler based on total time.\n");
    }

    // Clean up test file if created
    if (use_workload) {
        unlink(test_file);
    }

    exit();
}