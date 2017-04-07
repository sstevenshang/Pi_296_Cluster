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
#include <unistd.h>

//Our headers
#include "master.h"
#include "node.h"
//#include "networkManager.h"

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
double get_local_usage();

//heartbeat functions
int setUpUDPClient();
int setUpUDPServer();
void cleanupUDPSocket(int socket_fd);
void heartbeat(char* destinationAddr, char* destinationPort, int* alive);
int sendHeartbeat(int socket_fd, char* destinationAddr, char* destinationPort);
void cleanupUDPSocket(int socket_fd);
// void listenToHeartbeat(int* stethoscope);
#endif
