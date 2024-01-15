#define _GNU_SOURCE
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <execinfo.h>

#ifdef DEBUG
  #define debug(fn) fn
#else
  #define debug(fn)
#endif

pthread_mutex_t channel_lock = PTHREAD_MUTEX_INITIALIZER;

static int (*pthread_mutex_lock_cp)(pthread_mutex_t*);
static int (*pthread_mutex_unlock_cp)(pthread_mutex_t*);

int write_bytes(int fd, void *buf, size_t len)
{
    char *p = (char *)buf ;
    size_t acc = 0 ;

    while (acc < len) {
        size_t written ;
        if ((written = write(fd, p, len - acc)) == -1) 
            return 1 ;
        p += written ;
        acc += written ;
    }
    return 0 ;
}

int pthread_mutex_lock(pthread_mutex_t * lock){
  int fd = open(".ddtrace", O_WRONLY | O_SYNC);
  if(fd == -1){
    printf("Error opening file: %s\n", strerror(errno));
    return 1;
  }
  // int (*pthread_mutex_lock_cp)(pthread_mutex_t*);
  // int (*pthread_mutex_unlock_cp)(pthread_mutex_t*);
  char * error;
  int return_value;

  pthread_mutex_lock_cp = dlsym(RTLD_NEXT, "pthread_mutex_lock");
  if((error = dlerror()) != 0x0) exit(EXIT_FAILURE);
  pthread_mutex_unlock_cp = dlsym(RTLD_NEXT, "pthread_mutex_unlock");
  if((error = dlerror()) != 0x0) exit(EXIT_FAILURE);

  void * arr[10] ;
  char ** stack ;

  size_t sz = backtrace(arr, 10) ;
  stack = backtrace_symbols(arr, sz) ;

  fprintf(stderr, "Stack trace\n") ;
  fprintf(stderr, "============\n") ;
  for (int i = 0 ; i < sz ; i++)
          fprintf(stderr, "[%d] %s\n", i, stack[i]) ;
  fprintf(stderr, "============\n\n") ;

  char *p = stack[1] ;
	while (*p != '+' && *p) {
		p++ ;
	}
	if (*p == '+') {
		p++ ;
	}
	char *line_addr_str = p ;
	while (*p != ')' && *p) {
		p++ ;
	}
	*p = '\0' ;
  //! test
  printf("line_addr_str1: %s\n",line_addr_str);
  intptr_t line_addr = strtoul(line_addr_str,NULL,0);
  line_addr -= 4;
  sprintf(line_addr_str, "%p", (void *)line_addr);
  printf("line_addr_str2: %s\n",line_addr_str);

  pthread_t pid = pthread_self();
  
  pthread_mutex_lock_cp(&channel_lock);
  write_bytes(fd, &pid, sizeof(pid));
  write_bytes(fd, &lock, sizeof(lock));
  int line_addr_size = strlen(line_addr_str);
  write_bytes(fd, &line_addr_size, sizeof(line_addr_size));
  write_bytes(fd, line_addr_str, strlen(line_addr_str));
  pthread_mutex_unlock_cp(&channel_lock);

  //!test
  fprintf(stderr, "pthread_lock(%lu)=%p\n", pid, lock) ;
  close(fd);
  return_value = pthread_mutex_lock_cp(lock);

  return return_value;
}


int pthread_mutex_unlock(pthread_mutex_t * lock){
  int fd = open(".ddtrace", O_WRONLY | O_SYNC);
  if(fd == -1){
    printf("Error opening file: %s\n", strerror(errno));
    return 1;
  }
  // int (*pthread_mutex_lock_cp)(pthread_mutex_t*);
  // int (*pthread_mutex_unlock_cp)(pthread_mutex_t*);
  char * error;
  int return_value;

  pthread_mutex_lock_cp = dlsym(RTLD_NEXT, "pthread_mutex_lock");
  if((error = dlerror()) != 0x0) exit(EXIT_FAILURE);
  pthread_mutex_unlock_cp = dlsym(RTLD_NEXT, "pthread_mutex_unlock");
  if((error = dlerror()) != 0x0) exit(EXIT_FAILURE);

  void * arr[10] ;
  char ** stack ;

  size_t sz = backtrace(arr, 10) ;
  stack = backtrace_symbols(arr, sz) ;

  fprintf(stderr, "Stack trace\n") ;
  fprintf(stderr, "============\n") ;
  for (int i = 0 ; i < sz ; i++)
          fprintf(stderr, "[%d] %s\n", i, stack[i]) ;
  fprintf(stderr, "============\n\n") ;

  char *p = stack[1] ;
	while (*p != '+' && *p) {
		p++ ;
	}
	if (*p == '+') {
		p ++ ;
	}
	char *line_addr_str = p ;
	while (*p != ')' && *p) {
		p++ ;
	}
	*p = '\0' ;
  //! test
  printf("line_addr_str1: %s\n",line_addr_str);
  intptr_t line_addr = strtoul(line_addr_str,NULL,0);
  line_addr -= 4;
  sprintf(line_addr_str, "%p", (void *)line_addr);
  printf("line_addr_str2: %s\n",line_addr_str);


  pthread_t pid = pthread_self();

  pthread_mutex_lock_cp(&channel_lock);
  write_bytes(fd, &pid, sizeof(pid));
  write_bytes(fd, &lock, sizeof(lock));
  int line_addr_size = strlen(line_addr_str);
  write_bytes(fd, &line_addr_size, sizeof(line_addr_size));
  write_bytes(fd, line_addr_str, strlen(line_addr_str));
  pthread_mutex_unlock_cp(&channel_lock);
  //! test
  fprintf(stderr, "pthread_unlock(%lu)=%p\n", pid, lock) ;

  close(fd);
  return_value = pthread_mutex_unlock_cp(lock);
  return return_value;
}