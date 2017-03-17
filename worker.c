#include "worker.h"
/* SERVER FUNCTIONS */
int running = 0;
char* defaultPort = "9001";
char* defaultMaster = "sp17-cs241-005.cs.illinois.edu";
int socketFd = -1;




int server_main() {
    char tempBuf[100];
    fprintf(stdout, "Type address to connect (press enter to connect to default master: %s)\n", defaultMaster);
    size_t bytesRead = read(fileno(stdin), tempBuf, 99);
    tempBuf[bytesRead] = '\0';
    if(bytesRead == 1){ strcpy(tempBuf, defaultMaster);}
    socketFd = setUpWorker(tempBuf, defaultPort);
    if(socketFd == -1){
      fprintf(stderr, "Failed connection, try again later\n");
      return 0;
    }
    running = 1;
	// Spwan thread for heartbeat
    while(running) {
		// Wait for incoming connection
		// Spwan thread to execute
    }
    cleanUpWorker(socketFd);
    return 0;
}



int setUpWorker(char* addr, char* port){
  int s;
  int socket_fd = socket(AF_INET, SOCK_STREAM |SOCK_NONBLOCK, 0);
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

int cleanUpWorker(int socket){
  shutdown(socket, SHUT_WR);

  return 0;
}

int getFdForWriteFile(char* name){
  int write_fd = open(name, O_CREAT | O_RDWR, S_IRWXG | S_IRWXU);
  if(write_fd == -1){
    fprintf(stderr, "failed to create write file %s\n", name);
    return -1;
  }
  return write_fd;
}


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
