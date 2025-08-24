#include "types.h"
#include "stat.h"
#include "user.h"

int main(int argc, char *argv[]) {
  if (argc < 2) {
    printf(1, "Usage: finddr <number>\n");
    exit();
  }
  int num = atoi(argv[1]);
  int dr = find_digit_root(num);
  printf(1, "Digital root of %d is %d\n", num, dr);
  exit();
}
