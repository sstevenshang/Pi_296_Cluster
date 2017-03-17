// #include "networkManager.h"
#include "heartbeat.h"
#include <time.h>
#include <pthread.h>

int master_main(node* this_node);
void reportHeartbeat(char* beat_addr);
void* listenToHeartbeatThread(void* load);
void* updateNodeStatusThread(void* load);
void resetBeats();
double getTime();
