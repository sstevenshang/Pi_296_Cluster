#ifndef _HEARTBEAT_H_
#define _HEARTBEAT_H_

#include <sys/socket.h>
#include <stdio.h>
#include <netinet/in.h>
#include <string.h>
#include <sys/types.h>
#include <utils.h>

#include "master.h"

int setUpUDPClient();

int setUpUDPServer();

void cleanupUDPSocket(int socket_fd);

void heartbeat(char* destinationAddr, char* destinationPort, int* alive);

int sendHeartbeat(int socket_fd, char* destinationAddr, char* destinationPort);

void listenToHeartbeat(int* stethoscope);

#endif
