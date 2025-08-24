#include "types.h"
#include "stat.h"
#include "user.h"

void printf(int fd, const char* format, ...);

int main() {
  printf(1, "Starting lifetime program...\n");
  int pid = getpid();
  sleep(69);
  int lifetime = get_process_lifetime(pid);
  printf(1, "Lifetime of process %d: %d ticks\n", pid, lifetime);
  exit();
}
