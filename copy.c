#include "types.h"
#include "stat.h"
#include "user.h"

// Implementation using existing system calls instead of a new copy_file syscall
int local_copy_file(char *src, char *dest) {
  int fd_src, fd_dest;
  char buf[512];
  int n;

  if((fd_src = open(src, 0)) < 0)
    return -1;
  
  if((fd_dest = open(dest, 0x200 | 0x002)) < 0) { // O_CREATE | O_WRONLY
    close(fd_src);
    return -1;
  }

  while((n = read(fd_src, buf, sizeof(buf))) > 0) {
    if(write(fd_dest, buf, n) != n) {
      close(fd_src);
      close(fd_dest);
      return -1;
    }
  }
  
  close(fd_src);
  close(fd_dest);
  return 0;
}

int
main(int argc, char *argv[])
{
  if(argc != 3){
    printf(1, "Usage: copy <src> <dest>\n");  // Add file descriptor 1 (stdout)
    exit();  // Remove argument
  }

  if(local_copy_file(argv[1], argv[2]) < 0){
    printf(1, "Copy failed\n");  // Add file descriptor 1
  } else {
    printf(1, "Copy succeeded\n");  // Add file descriptor 1
  }

  exit();  // Remove argument
}
