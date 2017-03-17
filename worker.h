#ifndef _WORKER_H_
#define _WORKER_H_


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
#include "node.h"
//#include "networkManager.h"
//#include "heartbeat.h"

extern int running;
extern int socketFd;
extern char* defaultMaster;
extern char* defaultPort;

//also ignoring heartbeat functions for now
int server_main();
int getFdForWriteFile(char* name);
int setUpWorker(char* addr, char* port);
int cleanUpWorker(int socket);
char* getBinaryFile(int socket, char* name);
void runBinaryFile(char* name);
void* threadManager(void* arg);
void resetPipeClient(int socket);
#endif
