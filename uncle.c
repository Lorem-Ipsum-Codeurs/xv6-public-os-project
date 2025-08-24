#include "types.h"
#include "stat.h"
#include "user.h"

int main() {
  int pid = getpid();  // Get current process PID
  int count = get_uncle_count(pid);
  printf(1, "Uncle count for PID %d = %d\n", pid, count);
  exit();
}
