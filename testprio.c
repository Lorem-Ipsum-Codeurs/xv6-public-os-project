#include "types.h"
#include "stat.h"
#include "user.h"

int main() {
  printf(1, "Setting priority to 10...\n");
  set_priority(10);
  printf(1, "Priority set. Process continues.\n");

  exit();
}
