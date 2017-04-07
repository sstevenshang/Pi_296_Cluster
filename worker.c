#include "worker.h"

#define BUFSIZE 1024
#define SERVER_PORT 1153
#define SERVER_IP4 "127.0.0.1"

/* SERVER FUNCTIONS */
int runningC = 0;
char* defaultClientPort = "9001";
char* defaultMaster = "sp17-cs241-005.cs.illinois.edu";
int socketFd = -1;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

int server_main() {
    char tempBuf[100];
    fprintf(stdout, "Type address to connect (press enter to connect to default master: %s)\n", defaultMaster);
    size_t bytesRead = read(fileno(stdin), tempBuf, 99);
    tempBuf[bytesRead] = '\0';
    if(bytesRead == 1){ strcpy(tempBuf, defaultMaster);}
    socketFd = setUpWorker(tempBuf, defaultClientPort);
    if(socketFd == -1){
      fprintf(stderr, "Failed connection, try again later\n");
      return 0;
    }
    runningC = 1;
	// Spwan thread for heartbeat
    while(runningC) {
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
  fprintf(stdout, "using %s and %s\n", addr, port);
  if(s != 0){
    fprintf(stderr, "failed to find %s at port %s\n", addr, port);
    return -1;
  }
  if(connect(socket_fd, result->ai_addr, result->ai_addrlen)==-1){
    if(errno != EINPROGRESS){
      perror("connect");
      return -1;
    }
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
  pthread_attr_t* myAttr = NULL;
  pthread_create(&myThread, myAttr, &threadManager, (void*) name);
  pthread_join(myThread, NULL);
}

void* threadManager(void* arg){
  execlp((char*) arg, (char*)arg, NULL,(char*)NULL);
  fprintf(stderr, "exec returned (bad)\n");
  return (void*)-1;
}

void resetPipeClient(int socket){
  write(socket, NULL, 0);
  read(socket, NULL, 0);
}

int setUpUDPServer() {

	int socket_fd = socket(AF_INET, SOCK_DGRAM, 0);
	if (socket_fd < 0) {
		perror("FAILED: unable to create socket");
		return -1;
	}
	struct sockaddr_in serverAddr;
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(SERVER_PORT);
	serverAddr.sin_addr.s_addr = inet_addr(SERVER_IP4);
	memset(serverAddr.sin_zero, 0, sizeof(serverAddr.sin_zero));

	int status = bind(socket_fd, (struct  sockaddr*) &serverAddr, sizeof(serverAddr));
	if (status < 0) {
		perror("FAILED: unable to bind socket");
		return -1;
	}

	return socket_fd;
}

void heartbeat(char* destinationAddr, char* destinationPort, int* alive) {
	int socket_fd = setUpUDPClient();
	while (*alive) {
    //Takes one second to compute
		while (sendHeartbeat(socket_fd, destinationAddr, destinationPort) == -1) {
			printf("Failed: failed to send heartbeat");
		}
	}
	cleanupUDPSocket(socket_fd);
}

int sendHeartbeat(int socket_fd, char* destinationAddr, char* destinationPort) {

	// char* message = "BEAT";
	double cpu_usage = get_local_usage();
	// size_t length = strlen(message) + 1;
	size_t length = sizeof(cpu_usage);

	struct sockaddr_in serverAddr;

	serverAddr.sin_family = AF_INET;
	serverAddr.sin_addr.s_addr = inet_addr(destinationAddr);
	serverAddr.sin_port = inet_addr(destinationPort);
	memset(serverAddr.sin_zero, 0, sizeof(serverAddr.sin_zero));

	socklen_t dest_len = sizeof(serverAddr);

	int status = sendto(socket_fd, &cpu_usage, length, 0, (struct sockaddr*) &serverAddr, dest_len);

	if (status < 0) {
		perror("FAILED: unable to send message to server");
		return -1;
	}

	return 0;
}

void listenToHeartbeat(int* stethoscope) {

	// unsigned char buffer[BUFSIZE];
	double client_usage;
	struct sockaddr_in clientAddr;
	socklen_t addrlen = sizeof(clientAddr);
	int byte_received = 0;
	int socket_fd = setUpUDPClient();

	while(*stethoscope) {

		byte_received = recvfrom(socket_fd, &client_usage, sizeof(client_usage), 0, (struct sockaddr*)&clientAddr, &addrlen);
		if (byte_received < 0) {
			perror("FAILED: failed to receive from client");
		} else {
			char* beat_addr = inet_ntoa(clientAddr.sin_addr);
			//char* beat_port = inet_ntoa(clientAddr.sin_port);
			reportHeartbeat(beat_addr, client_usage);
			printf("SUCCESS: received \"%f\" from %s\n", client_usage, beat_addr/*, beat_port*/);
		}
	}
	cleanupUDPSocket(socket_fd);
}
