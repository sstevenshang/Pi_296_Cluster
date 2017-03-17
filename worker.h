#ifndef _WORKER_H_
#define _WORKER_H_

#include "node.h"
#include "networkManager.h"
#include "heartbeat.h"

int server_main(node* this_node);
int getFdForWriteFile(char* name);
int setUpWorker(char* addr, char* port);
int cleanUpWorker(int socket);
char* getBinaryFile(int socket, char* name);
void runBinaryFile(char* name);
void* threadManager(void* arg);
void resetPipeClient(int socket);
#endif
