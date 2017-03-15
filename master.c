#include "node.h"
#include "networkManager.h"

/* MASTER FUNCTIONS */

int master_main(node* this_node, node_id** all_nodes_ids) {

	while(1) {

		// GET TASK INPUT

	}

	return 0;
}

typedef struct {
	node* this_node;
	node_id* server_id;
	char* file_to_send;
} request;

void* listenToHeartbeat(node* this_node, node_id** all_nodes_ids) {

}

node_id* findNodeWithLeastLoad(node* this_node, node_id** master_node_id) {
	return NULL;
}

int sendRequest(node* this_node, node_id* target) {
	return 0;
}

void* sendRequestHandler(void* load) {

	request* my_request = (request*)load;
	return NULL;
}


void reportHeartbeat(char* beat_addr, char* beat_port) {
	/* keep track of num of beat/skip balance
	   kill node if skip imbalanced
	   revive node if beat imbalanced*/

	//updateNodeStatus();
}