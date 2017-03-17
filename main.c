#include "node.h"
// #include "server.h"
#include "master.h"

#include <string.h>
#include <stdio.h>

//Need to write server.h, server.c, and this function server_main somewhere
void server_main(node* n) {
	(void) n;
}

int main(int argc, char const *argv[])
{
	node* this_node;
	this_node = construct_nodes();
	if (this_node->is_master == IS_MASTER) {
		master_main(this_node);
	} else {
		server_main(this_node);
	}

	return 0;
}
