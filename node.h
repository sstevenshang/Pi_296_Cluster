#include <string.h>
#include <sys/types.h>
#include <ifaddrs.h>

#define IS_MASTER 1
#define IS_WORKER 0

#define IS_ALIVE 1
#define IS_DEAD 0

#define IDLE 0
#define UNAVALIBLE -1


typedef struct {

	char* node_addr;
	char* node_port;

	int socket;

	int alive;
	int cur_load;
	int is_master;

	double last_beat_received_time;

	struct node* next;

} node;

node* head;
node* cur_master;

size_t node_counts = 4;
char* node_addresses[4] = { "192.168.1.1", "192.168.1.2", "192.168.1.3", "192.168.1.4" };
char* node_ports[4] = { "9001", "9001", "9001", "9001" };
char* default_master_address = "192.168.1.1";

node* construct_nodes();
int is_equal_address(char* a, char* b);
node* node_constructor(char* address, char* port);
