#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#define NUM_THREAD 10
#define NUM_MUTEX 10

int read_bytes(int fd, void *buf, size_t len)
{
    // return 0 if all given bytes are succesfully read
    // return 1 otherwise

    char *p = (char *)buf ;
    size_t acc = 0 ;

    while (acc < len) {
      size_t received ;
      if ((received = read(fd, p, len-acc)) == -1)
          return 1 ;
      p += received ;
      acc += received ;
    }

    return 0 ;
}

int main(int argc, char* argv[]){
  char command[512];
  char result[512];
  int line_number;

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
  char *line_addr;

  int len;

  while(1){
    // printf("check\n");
    read_bytes(fd, &pid, sizeof(pid));
    read_bytes(fd, &lock, sizeof(lock));
    int line_addr_size;
    read_bytes(fd, &line_addr_size, sizeof(line_addr_size));
    line_addr = (char *)malloc(line_addr_size);
    read_bytes(fd, line_addr, line_addr_size);
    // while(((len = read(fd, &pid, sizeof(pid))) == 0))
    // if(len == -1){
    //   perror("read from channel error\n");
    //   return 1;
    // }
    // if(len = read(fd, &lock, sizeof(lock)) == -1){
    //   perror("read from channel error\n");
    //   return 1;
    // }
    sprintf(command, "addr2line -e target %s",line_addr);
    FILE* fp = popen(command, "r");
    if (fp == NULL) {
        printf("Failed to run command\n" );
        return 1;
    }
    fgets(result, sizeof(result), fp);
    char * p = strrchr(result, ':');
    line_number = atoi(p + 1);

    printf("%lu : %p(%d)\n", pid, lock, line_number);
    // algorithm
    pclose(fp);
    free(line_addr);
  }
  close(fd) ;
  return 0 ;
}