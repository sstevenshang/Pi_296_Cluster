#ifndef _MASTER_H_
#define _MASTER_H_

#include "node.h"

#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <netdb.h>
#include <netinet/in.h>
#include <net/if.h>
#include <pthread.h>
#include <signal.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>
#include "taskManager.h"
extern int runningM;
extern int client_incoming_fd;

int master_main();
int addAnyIncomingConnections(int incomingFd, int mode);
int getFdForReadFile(char* name);
int setUpMaster(char* port);
int cleanUpMaster(int socket);
int sendBinaryFile(int socket, char* name);

void reportHeartbeat(char* beat_addr, double client_usage);
double getTime();
int setUpUDPServer();
void* listenToHeartbeat(void* keepalive);
void reportHeartbeat(char* beat_addr, double client_usage);

#endif
