#ifndef _WORKER_H_
#define _WORKER_H_

#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <netdb.h>
#include <netinet/in.h>
#include <pthread.h>
#include <signal.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>

#include "master.h"
#include "node.h"

extern int runningC;
extern int socketFd;
extern char* defaultMaster;
extern char* defaultPort;

//Non-heartbeat functions
int server_main();
int getFdForWriteFile(char* name);
int setUpWorker(char* addr, char* port);
int cleanUpWorker(int socket);
char* getBinaryFile(int socket, char* name);
void runBinaryFile(char* name);
void* threadManager(void* arg);
void resetPipeClient(int socket);

//heartbeat functions
void* spwan_heartbeat(void* load);
int setUpUDPClient();
void heartbeat(char* destinationAddr, char* destinationPort, int* alive);
int sendHeartbeat(int socket_fd, char* destinationAddr, char* destinationPort);
double get_local_usage();
char* get_local_addr();

#endif
