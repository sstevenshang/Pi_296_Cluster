#include "node.h"
#include "networkManager.h"
#include "heartbeat.h"

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

	/* keep track of num of beat/skip balance
	   kill node if skip imbalanced
	   revive node if beat imbalanced*/
	node* temp = traverseNodes(beat_addr);
	temp->beat++;

	//updateNodeStatus();
}

void updateNodeStatus() {

	int count = 0;
	while(1) {

	}

}
