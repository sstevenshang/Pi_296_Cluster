#include "master.h"

/* MASTER FUNCTIONS */

int master_main(node* this_node) {

	// Spawn thread for heartbeat
	pthread_t heartbeat_thread;
	pthread_t stethoscope_thread;

	int listen = 1;

	pthread_create(&heartbeat_thread, NULL, listenToHeartbeatThread, &listen);
	pthread_create(&stethoscope_thread, NULL, updateNodeStatusThread, &listen);

	updateNodeStatusThread(&listen);

	while(1) {
		// Wait for task input
		// Distribute task
		// Spawn thread to send request
	}

	pthread_join(heartbeat_thread, NULL);
	pthread_join(stethoscope_thread, NULL);

	return 0;
}

void reportHeartbeat(char* beat_addr) {
	double time_received = getTime();
	node* temp = traverseNodes(beat_addr);
	temp->last_beat_received_time = time_received;
}

void* listenToHeartbeatThread(void* listen) {
	listenToHeartbeat((int*)listen);
	return NULL;
}


void* updateNodeStatusThread(void* listen) {

	resetBeats();
	double time;
	while(*((int*)listen)) {
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
	return NULL;
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
