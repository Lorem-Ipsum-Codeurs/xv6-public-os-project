#include "types.h"
#include "stat.h"
#include "user.h"

int main() {
  int pid = fork();
  if(pid == 0){
    for(;;)
      printf(1, "Child1\n");
  }
  else{
    int pid2 = fork();
    if(pid2 == 0){
      for(;;)
        printf(1, "Child2\n");
    }
    else{
      for(;;)
        printf(1, "Parent\n");
    }
  }
  exit();  // Required in xv6
}
