#ifndef _MASTER_H_
#define _MASTER_H_

//#include "networkManager.h"
//#include "heartbeat.h"
#include <time.h>
#include <pthread.h>
#include "node.h"
#include <unistd.h>

//I'm ignoring heartbeat focused functions until I get everything else working -A
int master_main(node* this_node);
//void reportHeartbeat(char* beat_addr);
//void* listenToHeartbeatThread(void* load);
//void* updateNodeStatusThread(void* load);
//void resetBeats();
//double getTime();

#endif
