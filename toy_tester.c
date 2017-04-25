#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <string.h>

static size_t exec_num = 0;

void* threadManager(void* arg){
  pthread_detach(pthread_self());
  char buf[4096];
  sprintf(buf, "%s &> output%zu", (char*) arg, exec_num);
  if (system(buf) == -1)
    fprintf(stderr, "bad executable\n");
  //Now we need to send this output
  return (void*)-1;
}

void runBinaryFile(char* name){
  if(name == NULL){ fprintf(stderr, "trying to exec a null, stopping that\n"); exit(0);}
  pthread_t myThread;
  pthread_attr_t* myAttr = NULL;
  exec_num++;
  pthread_create(&myThread, myAttr, &threadManager, (void*) name);
}

int main() {
  runBinaryFile("./test");
  runBinaryFile("./test");
  pthread_exit(NULL);
  return 0;
}
