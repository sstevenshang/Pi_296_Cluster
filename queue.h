#ifndef _QUEUE_H_
#define _QUEUE_H_
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "node.h"

typedef struct queue_node {
	//TODO let's move to a task struct. See node.h.
	task* data;
	struct queue_node* next;
} queue_node;

typedef struct queue {
	queue_node *head, *tail;
	size_t size;
} queue;

queue* queue_create();
void queue_destroy(queue* this);
void queue_push(queue* this, task* element);
task* queue_pull(queue* this);
int queue_empty(queue* this);
size_t queue_size(queue* this);

#endif
