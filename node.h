
typedef struct {

	char* node_addr;
	char* node_port;

	char* master_addr;
	char* master_port;

	int socket;

	int alive;
	int cur_load;

	struct node* next;

} node;