#ifndef _MASTER_H_
#define _MASTER_H_

//#include "networkManager.h"
//#include "heartbeat.h"
#include <time.h>
#include <pthread.h>
#include "node.h"
#include <unistd.h>

//I'm ignoring heartbeat focused functions until I get everything else working -A
int master_main();
int addAnyIncomingConnections();
//void slaveManager();
int getFdForReadFile(char* name);
int setUpMaster(char* addr, char* port);
int cleanUpMaster(int socket);
int sendBinaryFile(int socket, char* name);
//void resetPipeServer(int socket);
//void reportHeartbeat(char* beat_addr);
//void* listenToHeartbeatThread(void* load);
//void* updateNodeStatusThread(void* load);
//void resetBeats();
//double getTime();

#endif
