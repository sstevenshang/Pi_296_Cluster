#include "networkManager.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <signal.h>
#include <errno.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <pthread.h>
#include <unistd.h>

int client_incoming_fd = -1;

typedef struct slave {
  int running; //0 if waiting, 1 if busy
  int socket;
  int taskNo; //if running = 0, taskNo == -1
  int taskPos;
  int bufSize;
  void* buf;
  struct slave* next;
} slave;

slave* headSlave = NULL;
slave* endSlave = NULL;
//returns -1 if invalid message header, otherwise returns the byte position
int getMessageType(char* header){
  unsigned char headerByte = *header;
  int type = 7;
  while(type){
    if(headerByte & 0x01){
      if(headerByte >>1 == 1 << (type-1)){
	return type;
      }
	return -1;
    }
    headerByte = headerByte >> 1;
    type --;
  }
  return 0;
}








//returns fd of socket;
//returns -1 if failed to create
int setUpClient(char* addr, char* port){
  int s;
  int socket_fd = socket(AF_INET, SOCK_STREAM, 0);
  struct addrinfo hints, *result;
  memset(&hints, 0, sizeof(struct addrinfo));
  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_STREAM;
  s = getaddrinfo(addr, port, &hints, &result);
  if(s != 0){
    fprintf(stderr, "failed to find %s at port %s\n", addr, port);
    return -1;
  }
  if(connect(socket_fd, result->ai_addr, result->ai_addrlen)==-1){
    perror("connect");
    return -1;
  }
  fprintf(stdout, "Found connection on fd %d\n", socket_fd);
//  resetPipeClient(socket_fd);
  return socket_fd;
}


int cleanUpClient(int socket){
  shutdown(socket, SHUT_WR);

  return 0;
}
// returns fd of file ready to write
//returns -1 if failed to create
int getFdForWriteFile(char* name){
  int write_fd = open(name, O_CREAT | O_RDWR, S_IRWXG | S_IRWXU);
  if(write_fd == -1){
    fprintf(stderr, "failed to create write file %s\n", name);
    return -1;
  }
  return write_fd;
}
//returns name of exec file that is runable by exec()
//returns NULL if any errors
char* getBinaryFile(int socket, char* name){
  int file = getFdForWriteFile(name);
  if(file == -1){ return NULL;}
  int bufSize = 2048;
  char buf[bufSize]; // completely abritray, could be up to 65535 (2^16 -1)
  int len = read(socket, buf, bufSize); //holds if no data in pipeline
  while(len != 0){ //len read something
    if(len == -1){
      fprintf(stderr, "some error with reading from the socket %d, check for SIGPIPE\n", socket);
      return NULL;
    }
    int s = write(file, buf, len);
    if(s == -1){ close(file); return NULL;}
    while(s != len){ // as long as write hasn't written all bytes
      int k = write(file, buf + s, len - s);
      if(k == -1){ close(file); return NULL;}
      s += k;
    }
    s = write(socket, NULL, 0); // nothing, give me more
    len = read(socket, buf, bufSize);
  }
  if(len == -1){
    fprintf(stderr, "some error with reading from the socket %d, check for SIGPIPE\n", socket);
    return NULL;
  }
  close(file);
  return name;
}

void runBinaryFile(char* name){
  if(name == NULL){ fprintf(stderr, "trying to exec a null, stopping that\n"); exit(0);}
  pthread_t myThread;
  pthread_attr_t* myAttr;
  pthread_create(&myThread, myAttr, &threadManager, (void*) name);
  pthread_join(myThread, NULL);
}

void* threadManager(void* arg){
  execlp((char*) arg, NULL, NULL);
  fprintf(stderr, "exec returned (bad)\n");
  return (void*)-1;
}

void resetPipeClient(int socket){
  write(socket, NULL, 0);
  read(socket, NULL, 0);
}


//returns fd of socket TO READ AND WRITE
//returns -1 in error
int setUpServer(char* addr, char* port){
  int s;
  int socket_fd = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0);
  struct addrinfo hints, *result;
  memset(&hints, 0, sizeof(struct addrinfo));
  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = AI_PASSIVE;
  s = getaddrinfo(addr, port, &hints, &result);
  if(s != 0){
    fprintf(stderr, "failed to find %s at port %s\n", addr, port);
    return -1;
  }
  int optval = 1;
  setsockopt(socket_fd, SOL_SOCKET, SO_REUSEPORT, &optval, sizeof(optval));
  if(bind(socket_fd, result->ai_addr, result->ai_addrlen) != 0) {
      perror("bind()");
      return -1;
  }
  if(listen(socket_fd, 10) != 0) {
      perror("listen()");
      return -1;
    }
  struct sockaddr_in *result_addr = (struct sockaddr_in*)result->ai_addr;
  client_incoming_fd = socket_fd;
  return 1;
 /* fprintf(stdout, "waiting for connection on port %s\n", port);
  int client_fd = accept(socket_fd, NULL, NULL);
  if(client_fd == -1){
    fprintf(stderr, "failed on waiting for client to connect\n");
    return -1;
  }
  fprintf(stdout, "connection found\n");
  resetPipeServer(client_fd);
  return client_fd;
  */
}
//returns -1 when no new connections, otherwise returns fd of pipe
int addAnyIncomingConnections(){
  int client_fd = accept(client_incoming_fd, NULL, NULL);
  if(client_fd != -1){  //found a connection
    slave* tmp = malloc(sizeof(slave));
    tmp->socket = client_fd;
    tmp->running = 1;
    tmp->next = NULL;
    tmp->taskNo = 0;
    tmp->taskPos = -1;
    tmp->bufSize = 2048;
    tmp->buf = (void*)malloc(tmp->bufSize);
    slave* oldEnd = endSlave;
    oldEnd->next = tmp;
    endSlave = tmp;
    return client_fd;
  }
  return client_fd;
}


int cleanUpServer(int socket){
  shutdown(socket, SHUT_WR);
  return 0;
}

void slaveManager(){
    slave* cur = headSlave;
    int bufSize = 2048;
    char buf[bufSize];
    while(cur != NULL){
      if(cur->running){
        if(cur->taskNo == -1){
          fprintf(stderr, "You messed up somewhere setting a taskNo to -1 with the slave at address %p\n", (void*)cur);
	  
        } else if(cur->taskNo == 0){ //taskNo 0 is slave setup;
          size_t bytesRead = read(cur->socket,  buf, bufSize);
          if(bytesRead == (size_t)(-1)){
            if(errno == EWOULDBLOCK){//returned -1 becuase there was nothing to read, but no block.

            } else {
              fprintf(stderr, "Invalid read from slave at address %p\n", (void*)cur);
            }
          } else {
            while(getMessageType(buf) != 2){
              bytesRead = bytesRead + read(cur->socket,  buf+bytesRead, bufSize - bytesRead);
            }
          }
        }



      }
      cur = cur->next;
    }
}


//returns fd for read in exec to send
//returns -1 on error
int getFdForReadFile(char* name){
  FILE * read_fd = fopen(name, "rb");
  if(read_fd == NULL){
    fprintf(stderr, "Error opening file %s\n", name);
    return -1; 
  }
  return fileno(read_fd);
}
//returns 0 on clean exit
// else returns -1
int sendBinaryFile(int socket, char* name){
  int file = getFdForReadFile(name);
  if(file == -1){ return -1;}
  int bufSize = 2048;
  char buf[bufSize]; // completely abritray, could be up to 65535 (2^16 -1)
  int len = read(file, buf, bufSize); //holds if no data in pipeline
  while(len != 0){ //len read something
    if(len == -1){
      fprintf(stderr, "some error with reading from the fd %d, check for SIGPIPE\n", file);
      return 0;
    }
    int s = write(socket, buf, len);
    if(s == -1){ close(file); return 0;}
    while(s != len){ // as long as write hasn't written all bytes
      int k = write(socket, buf + s, len - s);
      if(k == -1){ close(file); return 0;}
      s += k;
    }
    s = read(socket, NULL, 0); // ingnoring, putting more data
    len = read(file, buf, bufSize);
  }
  if(len == -1){
    fprintf(stderr, "some error with reading from the fd %d, check for SIGPIPE\n", file);
    return 0;
  }
  close(file);
  return 0;
}

void resetPipeServer(int socket){
  read(socket, NULL, 0);
  write(socket, NULL, 0);
}
