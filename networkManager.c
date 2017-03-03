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
    s = write(socket, NULL, NULL); // nothing, give me more
    len = read(socket, buf, bufSize);
  }
  if(len == -1){
    fprintf(stderr, "some error with reading from the socket %d, check for SIGPIPE\n", socket);
    return NULL;
  }
  close(file);
  return name;
}
//returns fd of socket TO READ AND WRITE
//returns -1 in error
int setUpServer(char* addr, char* port){
  int s;
  int socket_fd = socket(AF_INET, SOCK_STREAM, 0);
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
  fprintf(stdout, "waiting for connection on port %s\n", port);
  int client_fd = accept(socket_fd, NULL, NULL);
  if(client_fd == -1){
    fprintf(stderr, "failed on waiting for client to connect\n");
    return -1;
  }
  fprintf(stdout, "connection found\n");
  return client_fd;
}


int cleanUpServer(int socket){
  shutdown(socket, SHUT_WR);
  return 0;
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
      return NULL;
    }
    int s = write(socket, buf, len);
    if(s == -1){ close(file); return NULL;}
    while(s != len){ // as long as write hasn't written all bytes
      int k = write(socket, buf + s, len - s);
      if(k == -1){ close(file); return NULL;}
      s += k;
    }
    s = read(socket, NULL, NULL); // ingnoring, putting more data
    len = read(file, buf, bufSize);
  }
  if(len == -1){
    fprintf(stderr, "some error with reading from the fd %d, check for SIGPIPE\n", file);
    return NULL;
  }
  close(file);
  return 0;


}
