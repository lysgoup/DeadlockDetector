#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#define LOCK 1
#define UNLOCK 0
#define NUM_THREAD 10
#define NUM_MUTEX 10

#ifdef DEBUG
  #define debug(fn) fn
#else
  #define debug(fn)
#endif

typedef struct mythread_t{
  pthread_t thread_id;
  int thread_mutex[NUM_MUTEX];
}mythread_t;

typedef struct mymutex_t{
  struct mymutex_t *next;
  pthread_mutex_t * addr;
  int thread_index;
  char *line_addr;
}mymutex_t;

int graph[NUM_MUTEX][NUM_MUTEX];
mythread_t mythread[NUM_THREAD];
mymutex_t *mymutex[NUM_MUTEX];
pthread_mutex_t *mutex_dic[NUM_MUTEX];

int Stack[NUM_MUTEX];
int visited[NUM_MUTEX];
int parent[NUM_MUTEX];
int cycle = 0;

int read_bytes(int fd, void *buf, size_t len);
void update_graph(int mutex_index, int thread_index, int flag);
void show_graph();
int DFS(int v);
int check_cycle(int start, char *file);
void check_deadlock(pthread_t pid, pthread_mutex_t *lock, char *line_addr, char *file);

int read_bytes(int fd, void *buf, size_t len)
{
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

void update_graph(int mutex_index, int thread_index, int flag){
  for(int i=0;i<NUM_MUTEX;i++){
    if(mythread[thread_index].thread_mutex[i] == 1){
      if(mutex_index != i) graph[i][mutex_index] = flag;
    }
  }
}

void show_graph(){
  for(int i=0;i<=NUM_MUTEX;i++){
    for(int j=0;j<=NUM_MUTEX;j++){
      if(i==0 && j==0){
        printf("  ");
      }
      else if(i==0){
        printf("%d ",j-1);
      }
      else if(j==0){
        printf("%d ",i-1);
      }
      else{
        printf("%d ",graph[i-1][j-1]);
      }
    }
    printf("\n");
  }
}

int DFS(int v) {
  if(!visited[v]) {
    visited[v] = 1;
    Stack[v] = 1;

    for(int i = 0; i < NUM_MUTEX; i++) {
      if(graph[v][i]) {
        parent[i] = v;
        if(visited[i]==0){
          int c = DFS(i);
          if(c != 0) return c;
        }
        else if(Stack[i])
          return i;
      }
    }
  }
  Stack[v] = 0;
  return 0;
}

void print_cycle(int s, char *file){
  char result[1024];
  char command[512];
  int v;
  v = s;
  printf("-------------------Below thread, mutex are involved in the deadlock------------------\n");
  while(1){
    sprintf(command, "addr2line -e target %s",mymutex[v]->line_addr);
    FILE* fp = popen(command, "r");
    if (fp == NULL) {
        printf("Failed to run command\n" );
        return;
    }
    fgets(result, sizeof(result), fp);
    char * p = strrchr(result, ':');
    int line_number = atoi(p + 1);
    pclose(fp);
    sprintf(result, "| Thread ID: %lu, Mutex memory address: %p, Line number: %d |",mythread[mymutex[v]->thread_index].thread_id, mymutex[v]->addr, line_number);
    printf("%s\n",result);
    v = parent[v];
    if(v == s) break;
  }
  printf("-------------------------------------------------------------------------------------\n");
}

int check_cycle(int start, char *file){
  for(int i = 0; i < NUM_MUTEX; i++) {
    visited[i] = 0;
    Stack[i] = 0;
    parent[i] = -1;
  }
  int has_cycle = DFS(start);
  if(has_cycle != 0){
    print_cycle(has_cycle, file);
    return 1;
  }
  else return 0;
}

void check_deadlock(pthread_t pid, pthread_mutex_t *lock, char *line_addr, char *file){
  int i, j;
  //mutex indexing: index는 i
  for(i=0;i<NUM_MUTEX;i++){
    if(mutex_dic[i] == lock){
      break;
    }
  }
  if(i==NUM_MUTEX){
    for(i=0;i<NUM_MUTEX;i++){
      if(mutex_dic[i] == NULL){
        mutex_dic[i] = lock;
        debug(printf("make new mutex: %d\n",i);)
        break;
      }
    }
  }

  //현재 쓰레드가 mythread배열에 존재하는지 확인: 존재하면 j가 index
  for(j=0;j<NUM_THREAD;j++){
    if(mythread[j].thread_id == pid) break;
  }

  //현재 쓰레드가 배열에 존재하지 않으면 새로 만든다. thread의 index는 j가 됨
  if(j==NUM_THREAD){
    for(j=0;j<NUM_THREAD;j++){
      if(mythread[j].thread_id == 0){
        mythread[j].thread_id = pid;
        debug(printf("make new thread: %d\n",j);)
        break;
      }
    }
  }

  //mutex를 가지고 있는 쓰레드가 없을 때 -> mutex획득
  if(mymutex[i] == NULL){
    //mutex의 index는 i가 됨
    mymutex[i] = (mymutex_t*)malloc(sizeof(mymutex_t));
    mymutex[i]->addr = lock;
    mymutex[i]->next = NULL;
    mymutex[i]->thread_index = j;
    mymutex[i]->line_addr = line_addr;
    strcpy(mymutex[i]->line_addr, line_addr);
    
    //새로운 mutex를 획득함을 표시한다.
    mythread[j].thread_mutex[i] = 1;
    update_graph(i,mymutex[i]->thread_index,LOCK);
    debug(printf("thread %d get mutex %d\n",j,i);)
  }
  //mutex를 가지고 있는 쓰레드가 있을 때 (mymutex배열에 현재 mutex가 들어있을 때)
  else{
    //그 mutex를 가지고 있는 쓰레드가 현재 쓰레드이면 unlock
    if(mythread[j].thread_mutex[i] == 1){
      update_graph(i,mymutex[i]->thread_index,UNLOCK);
      debug(printf("thread %d unlock mutex %d\n",j,i);)
      mythread[j].thread_mutex[i] = 0;
      if(mymutex[i]->next == NULL){
        free(mymutex[i]->line_addr);
        free(mymutex[i]);
        mymutex[i] = NULL;
      }
      else{
        mymutex_t * temp;
        temp = mymutex[i];
        mymutex[i] = mymutex[i]->next;
        free(temp);
        mythread[mymutex[i]->thread_index].thread_mutex[i] = 1;
        debug(printf("thread %d get mutex %d\n",mymutex[i]->thread_index,i);)
      }
    }
    //현재 쓰레드가 그 mutex를 가지고 있지 않다면, linked list에 연결
    else{
      mymutex_t *newnode = (mymutex_t *)malloc(sizeof(mymutex_t));
      newnode->thread_index = j;
      newnode->addr = lock;
      newnode->next = NULL;
      newnode->line_addr = line_addr;
      mymutex_t * temp = mymutex[i];
      while(temp->next != NULL){
        temp = temp->next;
      }
      temp->next = newnode;
      update_graph(i,newnode->thread_index,LOCK);
      debug(printf("thread %d is waiting for mutex %d\n",j,i);)
    }
  }
  show_graph();
  if(check_cycle(i, file) == 1){
    printf("DeadLock Detected!!\n");
    exit(EXIT_FAILURE);
  }
}

int main(int argc, char* argv[]){
  char command[512];
  char result[512];
  int line_number;

  if (mkfifo(".ddtrace", 0666)) {
    if (errno != EEXIST) {
      perror("fail to open fifo: ") ;
      exit(1) ;
    }
  }

  int fd = open(".ddtrace", O_RDONLY | O_SYNC) ;
  // pid_t process_id;
  pthread_t pid;
  pthread_mutex_t * lock;

  int len;

  while(1){
    read_bytes(fd, &pid, sizeof(pid));
    read_bytes(fd, &lock, sizeof(lock));
    int line_addr_size;
    read_bytes(fd, &line_addr_size, sizeof(line_addr_size));
    char *line_addr;
    line_addr = (char *)malloc(line_addr_size);
    read_bytes(fd, line_addr, line_addr_size);
    
    //algorithm
    check_deadlock(pid, lock, line_addr, argv[1]);

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
    pclose(fp);
  }
  close(fd) ;
  return 0 ;
}