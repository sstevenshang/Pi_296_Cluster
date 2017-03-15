#include "node.h"
#include "networkManager.h"

/* SERVER FUNCTIONS */

int server_main(node* this_node, node_id** all_nodes_ids) {

	while(1) {

		// WAIT FOR TASK

	}

	return 0;
}

typedef struct {
	node* this_node;
	node_id* master_id;
	char* file_to_execute;
} task;

void* sentHeartbeat(void* load) {
	node* this_node = (node*) load;
	while(1) {
		sleep(5);
		
	}
}

int executeRequest(node* this_node, char* file_to_execute) {
	return 0;
}

void* executeRequestHandler(void* load) {
	task* my_task = (task*) load;
	return NULL;
}
