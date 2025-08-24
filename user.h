struct stat;
struct rtcdate;
struct sysinfo; // Add if you have sysinfo struct

// system calls
int fork(void);
int exit(void) __attribute__((noreturn));
int wait(void);
int pipe(int*);
int write(int, const void*, int);
int read(int, void*, int);
int close(int);
int kill(int);
int exec(char*, char**);
int open(const char*, int);
int mknod(const char*, short, short);
int unlink(const char*);
int fstat(int fd, struct stat*);
int link(const char*, const char*);
int mkdir(const char*);
int chdir(const char*);
int dup(int);
int getpid(void);
char* sbrk(int);
int sleep(int);
int uptime(void);
int get_uncle_count(int);
int getprocessinfo(int);
int waitx(int*, int*); // Add if not present
int setschedpolicy(int);
int set_priority(int); // Add this line if not present
int ps(void);  // Add ps system call declaration
int test_rr(int);  // Test RR with n processes
int set_burst_estimate(int); // Add this with the other system call declarations
int yield(void); // Add this with other system call declarations

// ulib.c
int stat(const char*, struct stat*);
char* strcpy(char*, const char*);
void *memmove(void*, const void*, int);
char* strchr(const char*, char c);
int strcmp(const char*, const char*);
void printf(int, const char*, ...);
char* gets(char*, int max);
uint strlen(const char*);
void* memset(void*, int, uint);
void* malloc(uint);
void free(void*);
int atoi(const char*);
int find_digit_root(int num); // New function to find the digit root of a number
int copy_file(char *src, char *dest); // New function to copy a file
int get_uncle_count(int pid); // New function to get the uncle count
int get_process_lifetime(int pid);
int getnumsyscalls(void);
int getnumsyscallsgood(void);



