#define O_RDONLY  0x000
#define O_WRONLY  0x001
#define O_RDWR    0x002
#define O_CREATE  0x200
#define O_TRUNC   0x400

// Scheduler policy defines
#define SCHED_FCFS   0
#define SCHED_SJF    1
#define SCHED_BJF    2
#define SCHED_RANDOM 3
#define SCHED_RR     4 // Default Round Robin
