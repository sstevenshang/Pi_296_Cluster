#include "server_main.h"
#include "master_main.h"
#include "node.h"

#include <string.h>

#define NODE_COUNT 4
#define NODE_ADDRESSES { "192.168.1.1", "192.168.1.2", "192.168.1.3", "192.168.1.4" }
#define NODE_PORTS { "9001", "9001", "9001", "9001" }
#define MASTER_ADDRESS "192.168.1.1"

int main(int argc, char const *argv[])
{
	// SET UP NODES

	node* this_node;

	node_id* all_node_ids[NODE_COUNT];
	node_id* master_node_id = NULL;

	char* addresses[NODE_COUNT] = NODE_ADDRESSES;
	char* ports[NODE_COUNT] = NODE_PORTS;

	for (size_t i=0; i<NODE_COUNT; i++) {
		all_node_ids[i] = node_id_constructor(address[i], ports[i]);
		if (!strcmp(addresses[i], MASTER_ADDRESS)) {
			master_node_id = all_nodes_ids[i];
		}
	}

	assert(master_node_id);
	setUpNode(this_node, master_node_id);

	// RUN SERVER_MAIN or MASTER_MAIN

	if (this_node->master_server_mode) {
		master_main(this_node, all_nodes_ids);
	} else {
		server_main(this_node, all_nodes_ids);
	}

	return 0;
}