#include "node.h"
// #include "server.h"
#include "master.h"
#include "worker.h"
#include <string.h>
#include <stdio.h>
#include <unistd.h>
//Need to write server.h, server.c, and this function server_main somewhere
/*void server_main(node* n) {
	(void) n;
}
*/
int main(int argc, char const *argv[])
{
//	node* this_node;
//	this_node = construct_nodes();
    char buf[100];
    fprintf(stdout,
        "Enter 0 to run as client, 1 to run as master, 2 to run as worker\n");
    size_t bytesRead = read(fileno(stdin), buf, 99);
    if (bytesRead != 2) { //input not "1\n"or "2\n"
        fprintf(stderr, "Error: input is not 0, 1 or 2, terminating\n");
        return 0;
    }
    buf[2] = '\0';
    if (strcmp("0\n", buf) == 0) {
        interface_main();
	} else if (strcmp("1\n", buf) == 0) {
		master_main();
	} else if(strcmp("2\n", buf) == 0) {
		server_main();
	} else {
        fprintf(stderr, "Error: input is not 0, 1 or 2, terminating %s\n", buf);
    }
	return 0;
}
