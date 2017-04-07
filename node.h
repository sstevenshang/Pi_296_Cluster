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

	struct node* next;
} node;

extern node* head;
extern node* lastInList;

void addNode(int socket_fd, char* address);
void removeNode(node* oldNode);
void cleanNode(node* to_free);
node* searchNodeByAddr(char* beat_addr);

#endif
