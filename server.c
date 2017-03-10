#include "node.h"
#include "networkManager.h"

/* SERVER FUNCTIONS */

typedef struct {
	node* this_node;
	node_id* master_id;
	char* file_to_execute;
} task;

void sentHeartbeat(node* this_node) {
}

int executeRequest(node* this_node, char* file_to_execute) {
	return 0;
}

void* executeRequestHandler(void* load) {
	task* my_task = (task*) load;
	return NULL;
}
