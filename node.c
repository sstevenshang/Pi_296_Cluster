#include "node.h"

/* the function initializes the node linked list that includes all nodes
 * and returns the corresponding local node.
 * the function is only called once at initialization.*/

/* TODO:
 * figure out what to do if the master goes down and comes back up.
 * */
/*
node* construct_nodes() {

    char* local_addr = get_local_address();
    node* local_node = NULL;
    node* temp = NULL;
    for (size_t i=0; i<node_counts; i++) {
        node* cur = node_constructor(node_addresses[i], node_ports[i]);
        cur->next = temp;
        temp = cur;
        if (is_equal_address(node_addresses[i], local_addr)) {
            local_node = cur;
        }
    }
    head = local_node;
    return local_node;
}

int is_equal_address(char* a, char* b) {
    return (!strcmp(a, b));
}

node* node_constructor(char* address, char* port) {
    node* this = malloc(sizeof(node));
    this->node_addr = strdup(address);
    this->node_port = strdup(address);
    if (is_equal_address(address, default_master_address)) {
        cur_master = this;
        this->is_master = IS_MASTER;
        this->socket = setUpClient();
    } else {
        this->is_master = IS_WORKER;
        this->socket = setUpServer();
    }
    this->alive = IS_ALIVE;
    this->cur_load = IDLE;
    this->last_beat_received_time = 0;
    return this;
}

node* traverseNodes(char* address) {
    node* temp = head;
    while (temp) {
        if (is_equal_address(address, temp->node_addr)) {
            return temp;
        }
        temp = temp->next;
    }
    return NULL;
}

*/

//starting here -Andrew

head = NULL;
lastInList = NULL;

void addNode(int socketFd){
  node* newNode = (node*)malloc(sizeof(node));
  newNode->socket_fd = socketFd;
  newNode->alive = 1;
  newNode->cur_load = 0;
  newNode->taskNo = 0;
  newNode->taskPos = 0;
  newNode->next = NULL;
  if(head == NULL){
    head = newNode;
    lastInList = newNode;
  } else {
    lastInList->next = newNode;
    lastInList = newNode;
  }
}

void removeNode(node* oldNode){
  if(head == oldNode){
    head = oldNode->next;
    free(oldNode);
  }
  node* holder = head;
  
  while(holder != NULL){
    if(holder->next == oldNode){
      holder->next = oldNode->next;
      free(oldNode);
      exit(0);
    }
    holder = holder->next;
  }
  fprintf(stderr, "You tried removing an invalid node address: %p\n", oldNode);
}
