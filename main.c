#include "node.h"
#include "server.h"
#include "master.h"

#include <string.h>
#include <stdio.h>

int main(int argc, char const *argv[])
{
	node_id* this_node;
	this_node = construct_nodes();
	if (this_node->master == IS_MASTER) {
		master_main(this_node);
	} else {
		server_main(this_node);
	}

	return 0;
}