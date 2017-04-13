#ifndef _NODE_H_
#define _NODE_H_

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <ifaddrs.h>
#include <sys/socket.h>

#define IS_MASTER 1
#define IS_WORKER 0

#define IS_ALIVE 1
#define IS_DEAD 0

#define IDLE 0
#define UNAVALIBLE -1

#define MAX_TASKS_PER_NODE 5;

//Struct to hold all prevelent information of a task. Feel free to add to it as needed.
typedef struct task {
	char* file_name;
	//0 for failed, 1 for succeeded, 2 for running
	char status;
} task;

typedef struct node {
	int socket_fd;
	int alive;
  int taskNo;
  int taskPos;

	//Current CPU usage of the node
	double cur_load;

	//Last recieved heartbeat
	double last_beat_received_time;

	//Ip address of the node
	char* address;

	//List of tasks running on the node, removed when task is completed
	task** task_list;

	struct node* next;
} node;

void addNode(int socket_fd, char *address, node **head);
void removeNode(node *oldNode, node **head);
void cleanNode(node *to_free);
void free_all_nodes();
node* searchNodeByAddr(char* beat_addr, node *head);

#endif
