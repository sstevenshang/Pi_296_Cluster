#include "node.h"
#include "networkManager.h"
#include "heartbeat.h"
#include <time.h>

/* MASTER FUNCTIONS */

int master_main(node* this_node) {

	// Spawn thread for heartbeat

	while(1) {
		// Wait for task input
		// Distribute task
		// Spawn thread to send request
	}
	return 0;
}

void reportHeartbeat(char* beat_addr) {
	double time_received = getTime();
	node* temp = traverseNodes(beat_addr);
	temp->last_beat_received_time = time_received;
}

void* listenToHeartbeatThread(void* load) {
	int socket_fd = setUpUDPClient();
	listenToHeartbeat(socket_fd, 1);
}


void* updateNodeStatusThread(void* load) {

	/* keep track of num of beat/skip balance
	   kill node if skip imbalanced
	   revive node if beat imbalanced*/

	resetBeats();
	double time;
	while(1) {
		node* temp = head;
		for (size_t i=0; i<node_counts; i++) {
			if (temp->alive) {
				time = getTime();
				if ((time - temp->last_beat_received_time) > 9) {
					temp->alive = IS_DEAD;
				}
				temp = temp->next;
			}
		}
		sleep(2);
	}
}

void resetBeats() {
		node* temp = head;
		for (size_t i=0; i<node_counts; i++) {
			double time = getTime();
			temp->last_beat_received_time = time;
			temp = temp->next;
		}
}

double getTime() {
  struct timespec t;
  clock_gettime(CLOCK_MONOTONIC, &t);
  return t.tv_sec + 1e-9 * t.tv_nsec;
}
