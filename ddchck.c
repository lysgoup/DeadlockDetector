#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>

int main(int argc, char* argv[]){
  if (mkfifo("channel", 0666)) {
    if (errno != EEXIST) {
      perror("fail to open fifo: ") ;
      exit(1) ;
    }
  }

  int fd = open("channel", O_RDONLY | O_SYNC) ;
  // pid_t process_id;
  pthread_t pid;
  pthread_mutex_t * lock;

  int len;

  while(1){
    while(((len = read(fd, &pid, sizeof(pid))) == 0))
    if(len == -1){
      perror("read from channel error\n");
      return 1;
    }
    if(len = read(fd, &lock, sizeof(lock)) == -1){
      perror("read from channel error\n");
      return 1;
    }
    printf("pthread_unlock(%lu)=%p\n", pid, lock) ;

    // algorithm
  }
  close(fd) ;
  return 0 ;
}