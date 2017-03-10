#include "node.h"

#define IS_MASTER 1
#define IS_SERVER 0

#define IDLE 0
#define WORKING 1

#include <string.h>

typedef struct {

	char* addr;
	char* port;

} node_id ;

node_id

typedef struct {

	node_id* my_id;
	node_id* master_node_id;

	int status;
	int socket;
	int master_socket;
	int master_server_mode;

} node;

node* head;
node* tail;

node_id* node_id_constructor(char* address, char* port) {
	node_id* new_elem = malloc(sizeof(node_id));
	new_elem->addr = strdup(address);
	new_elem->port = strdup(port);
	return new_elem;
}

node_id* node_id_copy_constructor(node_id* elem) {
	node_id* new_elem = malloc(sizeof(node_id));
	new_elem->addr = strdup(elem->addr);
	new_elem->port = strdup(elem->port);
	return new_elem;
}

void node_id_destructor(node_id* elem) {
	free(elem->addr);
	free(elem->port);
	free(elem);
}

int compare_node_id(node_id* a, node_id* b) {
	return (!strcmp(a->addr, b->addr));
}

// TODO
node_id* getLocalID() {
	node_id* local_id = malloc(sizeof(node_id));
	return local_id;
}

void setUpNode(node* this, node_id* master_node_id) {

	this = malloc(sizeof(node));
	this->my_id = getLocalID();

	this->master_node_id = node_id_copy_constructor(master_node_id);
	this->master_server_mode = compare_node_id(this->my_id, this->master_node_id)

	if (master_server_mode == IS_MASTER) {
		this->socket = setUpClient(this->my_id);
		this->master_socket = -1;
	} else {
		this->socket = setUpServer(this->my_id);
	}

	int status = IDLE;
}
