#include "types.h"
#include "stat.h"
#include "user.h"

int main() {
  printf(1, "Sleeping for 10 ticks...\n");
  sleep(10);

  int total = getnumsyscalls();
  int good = getnumsyscallsgood();

  printf(1, "Total syscalls made: %d\n", total);
  printf(1, "Good syscalls made: %d\n", good);

  exit();
}
