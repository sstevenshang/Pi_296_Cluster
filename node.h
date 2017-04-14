#ifndef _NODE_H_
#define _NODE_H_

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <ifaddrs.h>
#include <sys/socket.h>
#include <pthread.h>

#define IS_MASTER 1
#define IS_WORKER 0

#define IS_ALIVE 1
#define IS_DEAD 0

#define IDLE 0
#define UNAVALIBLE -1

#define MAX_TASKS_PER_NODE 5

#define CURRENTLY_RUNNING 2
#define ON_QUEUE 3

//Struct to hold all prevelent information of a task. Feel free to add to it as needed.
typedef struct task {
	char* file_name;
	//See the list of macros for status of a task
	char status;
} task;

typedef struct node {
	int socket_fd;
	int alive;
  	int taskNo;
  	int taskPos;
	void* buf; //for message
	size_t bufSize; //defaults to 4096, can change
	size_t bufPos; // start pos for writing/reading data for asyrcronous calls
	int bufWIP; // bool if buf contains an incomplete message
	//Current CPU usage of the node
	double cur_load;

	//Last recieved heartbeat
	double last_beat_received_time;

	//Ip address of the node
	char* address;

	//List of tasks running on the node, removed when task is completed
	task** task_list;

	int num_of_task;

	struct node* next;
} node;

void addNode(int socket_fd, char *address, node **head);
void removeNode(node *oldNode, node **head);
void cleanNode(node *to_free);
void free_all_nodes();
void destory_mutex();
node* searchNodeByAddr(char* beat_addr, node *head);
node* get_next(node* curr);
void set_last_beat_received_time(node* curr, double time);
void set_load(node* curr, double cpu_load);
void set_alive(node* curr, int alive);
double get_last_heartbeat_time(node* curr);
char* get_address(node* cur);
void set_next(node* prev, node* next);
int is_alive(node* cur);
int get_num_of_task(node* cur);
void set_num_of_task(node* cur, int num_of_task);
void free_task(task* elem);
#endif
