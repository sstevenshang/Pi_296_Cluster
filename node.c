#include "node.h"
#include <string.h>
#include <sys/types.h>
#include <ifaddrs.h>

#define IS_MASTER 1
#define IS_WORKER 0

#define IS_ALIVE 1
#define IS_DEAD 0

#define IDLE 0
#define UNAVALIBLE -1

size_t node_counts = 4;
char** node_addresses = { "192.168.1.1", "192.168.1.2", "192.168.1.3", "192.168.1.4" };
char** node_ports = { "9001", "9001", "9001", "9001" };
char* default_master_address = "192.168.1.1";

typedef struct {

	char* node_addr;
	char* node_port;

	int socket;
    int master;
	int alive;
	int cur_load;

	struct node* next;

} node;

node* head;
node* cur_master;

/* the function initializes the node linked list that includes all nodes
 * and returns the corresponding local node.
 * the function is only called once at initialization.*/

/* TODO:
 * figure out what to do if the master goes down and comes back up.
 * */

node_id* construct_nodes() {

    char* local_addr = get_local_address();
    node* local_node = NULL;
    node* temp = NULL;
    for (size_t i=0; i<node_count; i++) {
        node* cur = node_constructor(node_addresses[i], node_ports[i]);
        cur->next = temp;
        temp = cur;
        if (is_equal_address(node_addresses[i], local_addr)) {
            local_node = cur;
        } 
    }
    return local_node;
}

int is_equal_address(char* a, char* b) {
    return (!strcmp(a, b));
}

node_id* node_constructor(char* address, char* port) {
    node* this = malloc(sizeof(node));
    this->node_addr = strdup(address);
    this->node_port = strdup(address); 
    this->master = is_equal_address(address, default_master_address);
    this->
    this->alive = IS_ALIVE;
    this->cur_load = IDLE; 
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
	int load = 0;
}
