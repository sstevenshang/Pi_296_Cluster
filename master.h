#ifndef _MASTER_H_
#define _MASTER_H_

#include "node.h"

#include <time.h>
#include <pthread.h>
#include <unistd.h>
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

extern int runningM;
extern int client_incoming_fd;

int master_main();
int addAnyIncomingConnections();
int getFdForReadFile(char* name);
int setUpMaster(char* port);
int cleanUpMaster(int socket);
int sendBinaryFile(int socket, char* name);

#endif
