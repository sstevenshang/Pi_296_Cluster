#ifndef _MASTER_H_
#define _MASTER_H_

#include "node.h"

#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <netdb.h>
#include <pthread.h>
#include <signal.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>

extern int runningM;
extern int client_incoming_fd;

int master_main();
int addAnyIncomingConnections();
int getFdForReadFile(char* name);
int setUpMaster(char* port);
int cleanUpMaster(int socket);
int sendBinaryFile(int socket, char* name);

//Called by master to report the heartbeat, update usage statistics
void reportHeartbeat(char* beat_addr, double client_usage);

//Gets the current time
double getTime();

//Returns the fd for a UDP client
int setUpUDPClient();

#endif
