#include "master.h"

/* MASTER FUNCTIONS */

int runningM = 0;
int clientIncomingFd = -1;
char* defaultMasterPort = "9001";

int master_main() {
        setUpMaster(defaultMasterPort);
        runningM = 1;
        while(runningM == 1){
          addAnyIncomingConnections();
        }
        cleanUpMaster(clientIncomingFd);
	return 0;
}

int setUpMaster(char* port){
  int s;
  int socket_fd = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0);
  struct addrinfo hints, *result;
  memset(&hints, 0, sizeof(struct addrinfo));
  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = AI_PASSIVE;
  char* junk2; junk2 = NULL;
  s = getaddrinfo(junk2, port, &hints, &result);
  if(s != 0){
    fprintf(stderr, "failed to find at port %s\n", port);
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
  void* junk = result_addr + 1;
  junk ++;
  clientIncomingFd = socket_fd;
  fprintf(stdout, "adding fd %d\n", clientIncomingFd);
  return 1;
}

int addAnyIncomingConnections(){
  struct sockaddr_in clientname;
  size_t size = sizeof(clientname);
  int client_fd = accept(clientIncomingFd, (struct sockaddr *) &clientname, (socklen_t*) &size);
  if(client_fd != -1){
    char* client_address = strdup(inet_ntoa(clientname.sin_addr));
    fprintf(stdout, "got incoming connection from %s\n", client_address);
    addNode(client_fd, client_address);
    return client_fd;
  }
  return client_fd;
}

int cleanUpMaster(int socket){
  shutdown(socket, SHUT_WR);
  return 0;
}

int getFdForReadFile(char* name){
  FILE * read_fd = fopen(name, "rb");
  if(read_fd == NULL){
    fprintf(stderr, "Error opening file %s\n", name);
    return -1;
  }
  return fileno(read_fd);
}

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

// HEART_BEAT CODE

int setUpUDPServer(char* master_addr, char* master_port) {

  int socket_fd = socket(AF_INET, SOCK_DGRAM, 0);
  if (socket_fd < 0) {
    perror("FAILED: unable to create socket");
    return -1;
  }

  struct sockaddr_in serverAddr;
  serverAddr.sin_family = AF_INET;
  serverAddr.sin_port = htons(master_port);
  serverAddr.sin_addr.s_addr = inet_addr(master_addr);
  memset(serverAddr.sin_zero, 0, sizeof(serverAddr.sin_zero));

  int status = bind(socket_fd, (struct sockaddr*) &serverAddr, sizeof(serverAddr));
  if (status < 0) {
    perror("FAILED: unable to bind socket");
    return -1;
  }

  return socket_fd;
}


void listenToHeartbeat(void* keepalive) {

  double client_usage;

  struct sockaddr_in clientAddr;
  socklen_t addrlen = sizeof(clientAddr);
  int byte_received = 0;

  int socket_fd = setUpUDPServer();
  int keep_listenning = *((int*)keepalive);

  while(keep_listenning) {
    byte_received = recvfrom(socket_fd, &client_usage, sizeof(client_usage), 0, (struct sockaddr*)&clientAddr, &addrlen);
    if (byte_received < 0) {
      perror("FAILED: failed to receive from client");
    } else {
      char* beat_addr = inet_ntoa(clientAddr.sin_addr);
      reportHeartbeat(beat_addr, client_usage);
      printf("SUCCESS: received \"%f\" from %s\n", client_usage, beat_addr);
    }
  }
  close(socket_fd);
}

void reportHeartbeat(char* beat_addr, double client_usage) {
  double time_received = getTime();
  node* reported_node = searchNodeByAddr(beat_addr);
  if (reported_node) {
      reported_node->last_beat_received_time = time_received;
      reported_node->cur_load = client_usage;
  }
}

double getTime() {
  struct timespec t;
  clock_gettime(CLOCK_MONOTONIC, &t);
  return t.tv_sec + 1e-9 * t.tv_nsec;
}





