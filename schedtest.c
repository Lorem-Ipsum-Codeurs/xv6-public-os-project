#include "types.h"
#include "stat.h"
#include "user.h"
#include "fcntl.h" // Already included, make sure it's there

// Structure to store results for each scheduler
typedef struct {
    char name[20];       // Scheduler name
    int policy;          // Scheduler policy number
    int completed;       // Completed children
    int total_wtime;     // Total wait time
    int total_rtime;     // Total run time
    int avg_wtime;       // Average wait time
    int avg_rtime;       // Average run time
    int avg_turnaround;  // Average turnaround time
} scheduler_result;

// Function to run test with a specific scheduler
int run_scheduler_test(int policy, scheduler_result *result);

// Simple CPU-bound task placeholder (replace if find_digit_root exists)
int cpu_work(int num) {
    int result = 0;
    volatile int i, j; // Use volatile to prevent over-optimization
    for (i = 0; i < num; i++) {
        for (j = 0; j < 100000; j++) {
            result += (i * j) % 10;
            result %= 10000; // Keep result bounded
        }
    }
    return result;
}

// Simple I/O-bound task: copy file content - Renamed to avoid conflict
int schedtest_copy_file(char *src, char *dest) { // Renamed function
    int fds, fdd, n;
    char buf[512];

    if ((fds = open(src, O_RDONLY)) < 0) {
        printf(2, "schedtest: cannot open source %s\n", src);
        return -1;
    }
    if ((fdd = open(dest, O_CREATE | O_WRONLY)) < 0) {
        printf(2, "schedtest: cannot open dest %s\n", dest);
        close(fds);
        return -1;
    }

    while ((n = read(fds, buf, sizeof(buf))) > 0) {
        if (write(fdd, buf, n) != n) {
            printf(2, "schedtest: write error to %s\n", dest);
            close(fds);
            close(fdd);
            return -1;
        }
    }

    if (n < 0) {
        printf(2, "schedtest: read error from %s\n", src);
    }

    close(fds);
    close(fdd);
    return (n < 0 ? -1 : 0);
}

// Child process workload
void child_workload() {
    int my_pid = getpid();
    int uncles = -1; // Default if syscall fails or not used
    int start_syscalls = -1, end_syscalls = -1; // Uncommented
    int lifetime = -1; // Uncommented
    char dest_filename[32];
    char pid_str[12]; // Buffer for PID string conversion
    char *p;
    int temp_pid, i, j;

    start_syscalls = getnumsyscalls(); // Uncommented

    printf(1, "Child %d: Starting workload.\n", my_pid);

    // 1. CPU Work
    printf(1, "Child %d: Performing CPU work...\n", my_pid);
    cpu_work(my_pid % 5 + 1); // Vary workload slightly based on PID

    // 2. Get Uncle Count
    printf(1, "Child %d: Getting uncle count...\n", my_pid);
    uncles = get_uncle_count(my_pid); // Use your syscall

    // 3. I/O Work (Copy file)
    printf(1, "Child %d: Performing I/O work (file copy)...\n", my_pid);

    // Manually construct filename: "dest_PID.txt"
    // Convert PID to string (reverse order)
    temp_pid = my_pid;
    i = 0;
    if (temp_pid == 0) {
        pid_str[i++] = '0';
    } else {
        while (temp_pid > 0) {
            pid_str[i++] = (temp_pid % 10) + '0';
            temp_pid /= 10;
        }
    }
    pid_str[i] = '\0'; // Null terminate the reversed string

    // Reverse the PID string
    for (j = 0; j < i / 2; j++) {
        char tmp = pid_str[j];
        pid_str[j] = pid_str[i - 1 - j];
        pid_str[i - 1 - j] = tmp;
    }

    // Construct the final filename
    p = dest_filename;
    strcpy(p, "dest_");
    p += strlen("dest_");
    strcpy(p, pid_str);
    p += strlen(pid_str);
    strcpy(p, ".txt");
    // Ensure null termination (handled by the last strcpy)

    // printf(1, "Child %d: Using dest file: %s\n", my_pid, dest_filename); // Debug print

    // Call the renamed function
    if (schedtest_copy_file("src_copy_file.txt", dest_filename) == 0) {
        unlink(dest_filename); // Clean up the copied file
    } else {
        printf(2, "Child %d: File copy failed.\n", my_pid);
    }

    // 4. (Optional) Change priority mid-way (Example for BJF)
    #ifdef BJF // Only compile this if BJF is defined during build
    int new_priority = (my_pid % 2 == 0) ? 1 : 3; // Example: Alternate priority
    printf(1, "Child %d: Setting priority to %d\n", my_pid, new_priority);
    set_priority(my_pid, new_priority);
    cpu_work(1); // Do a little more work after priority change
    #endif

    end_syscalls = getnumsyscalls(); // Uncommented
    lifetime = get_process_lifetime(my_pid); // Uncommented

    // Uncommented optional parts in printf
    printf(1, "Child %d finished: Uncles=%d, Lifetime=%d, Syscalls Used=%d\n",
           my_pid, uncles, lifetime, end_syscalls - start_syscalls);
    exit(); // Child must exit
}

// Add this prototype after your child_workload() function and before main()
int numDigits(int n);  // Function prototype

int main(int argc, char *argv[]) {
    // Define our scheduler types
    scheduler_result results[5]; // Array for 5 scheduler types
    int num_schedulers = 5;
    
    // Initialize scheduler info
    strcpy(results[0].name, "FCFS");
    results[0].policy = SCHED_FCFS;
    
    strcpy(results[1].name, "SJF");
    results[1].policy = SCHED_SJF;
    
    strcpy(results[2].name, "BJF");
    results[2].policy = SCHED_BJF;
    
    strcpy(results[3].name, "Random");
    results[3].policy = SCHED_RANDOM;
    
    strcpy(results[4].name, "Round Robin");
    results[4].policy = SCHED_RR;

    printf(1, "=== Starting Scheduler Comparison Test ===\n");
    
    // Run test for each scheduler type
    for (int s = 0; s < num_schedulers; s++) {
        printf(1, "\n\n=== Testing %s Scheduler ===\n", results[s].name);
        
        // Set scheduler policy
        if (setschedpolicy(results[s].policy) < 0) {
            printf(2, "Failed to set scheduler policy to %s\n", results[s].name);
            continue;
        }
        
        // Run the test with this scheduler
        if (run_scheduler_test(results[s].policy, &results[s]) < 0) {
            printf(2, "Test failed for %s scheduler\n", results[s].name);
        }
    }
    
    // Print comprehensive comparison table
    printf(1, "\n\n=== SCHEDULER COMPARISON RESULTS ===\n");
    printf(1, "+----------------+----------------+----------------+----------------+\n");
    printf(1, "| Scheduler      | Avg Wait Time | Avg Run Time   | Avg Turnaround |\n");
    printf(1, "+----------------+----------------+----------------+----------------+\n");
    
    for (int s = 0; s < num_schedulers; s++) {
        if (results[s].completed > 0) {
            printf(1, "| %s", results[s].name);
            // Add spaces to align (16 - length of name)
            for (int sp = 0; sp < 14 - strlen(results[s].name); sp++) {
                printf(1, " ");
            }
            printf(1, " | %d", results[s].avg_wtime);
            // Add spaces to align (up to 14 characters)
            for (int sp = 0; sp < 14 - numDigits(results[s].avg_wtime); sp++) {
                printf(1, " ");
            }
            printf(1, " | %d", results[s].avg_rtime);
            // Add spaces to align
            for (int sp = 0; sp < 14 - numDigits(results[s].avg_rtime); sp++) {
                printf(1, " ");
            }
            printf(1, " | %d", results[s].avg_turnaround);
            // Add spaces to align
            for (int sp = 0; sp < 14 - numDigits(results[s].avg_turnaround); sp++) {
                printf(1, " ");
            }
            printf(1, " |\n");
        } else {
            printf(1, "| %s              | N/A            | N/A            | N/A            |\n", 
                   results[s].name);
        }
    }
    
    printf(1, "+----------------+----------------+----------------+----------------+\n");
    printf(1, "Scheduler comparison completed. Exiting.\n");
    exit();
}

// Add this helper function before main()
int numDigits(int n) {
    if (n == 0) return 1;
    int count = 0;
    if (n < 0) {
        count = 1;  // Count the negative sign
        n = -n;
    }
    while (n > 0) {
        count++;
        n /= 10;
    }
    return count;
}

// Add this function to run a test with the specified scheduler
int run_scheduler_test(int policy, scheduler_result *result) {
    int num_children = 5; // Number of child processes to create
    int pids[num_children];
    int i;
    int total_wtime = 0;
    int total_rtime = 0;
    int completed_children = 0;
    int fd;
    const char *src_filename = "src_copy_file.txt";
    const char *file_content = "This is the source file content for the copy test.\n";

    // 1. Setup: Create the source file
    printf(1, "Creating source file '%s'...\n", src_filename);
    unlink(src_filename); // Remove if it exists
    if ((fd = open(src_filename, O_CREATE | O_WRONLY)) < 0) {
        printf(2, "Failed to create source file.\n");
        return -1;
    }
    if (write(fd, file_content, strlen(file_content)) != strlen(file_content)) {
         printf(2, "Failed to write to source file.\n");
         close(fd);
         return -1;
    }
    close(fd);

    // 2. Forking Children
    printf(1, "Forking %d children...\n", num_children);
    for (i = 0; i < num_children; i++) {
        int pid = fork();
        if (pid < 0) {
            printf(2, "Fork failed.\n");
            // Clean up already created children
            for(int j = 0; j < i; j++) {
                kill(pids[j]);
                wait(); // Clean up zombies
            }
            return -1;
        } else if (pid == 0) {
            // Child process
            child_workload();
            // child_workload calls exit(), so this part is unreachable
        } else {
            // Parent process
            pids[i] = pid;
            printf(1, "Forked child %d with PID %d.\n", i, pid);

            // Set priority for BJF testing if needed
            if (policy == SCHED_BJF) {
                int initial_priority = (i % 3) + 1; // Example: 1, 2, 3, 1, 2...
                printf(1, "Setting initial priority for PID %d to %d\n", pid, initial_priority);
                set_priority(initial_priority);
            }
        }
    }

    // 3. Data Collection & Monitoring
    printf(1, "Waiting for children to complete...\n");
    ps(); // Show initial state

    while (completed_children < num_children) {
        int wtime, rtime;
        int ret_pid = waitx(&wtime, &rtime); // Wait for ANY child

        if (ret_pid > 0) {
            completed_children++;
            total_wtime += wtime;
            total_rtime += rtime;
            printf(1, "Child PID %d finished. Wait Time = %d, Run Time = %d\n", 
                   ret_pid, wtime, rtime);

            // Find which child index this was
            for(int j=0; j<num_children; j++) {
                if(pids[j] == ret_pid) {
                    pids[j] = -1; // Mark as completed
                    break;
                }
            }
        } else if (ret_pid == -1) {
             printf(2, "waitx error.\n");
             break;
        }
    }

    // Final ps() call
    printf(1, "\n--- Final Process Status ---\n");
    ps();
    printf(1, "---------------------------\n");

    // Store results for this scheduler
    result->completed = completed_children;
    result->total_wtime = total_wtime;
    result->total_rtime = total_rtime;
    
    if (completed_children > 0) {
        result->avg_wtime = total_wtime / completed_children;
        result->avg_rtime = total_rtime / completed_children;
        result->avg_turnaround = (total_wtime + total_rtime) / completed_children;
    }

    // Cleanup
    printf(1, "Cleaning up source file '%s'...\n", src_filename);
    unlink(src_filename);
    
    return 0;
}