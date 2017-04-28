#include "queue.h"
#include "node.h"

// typedef struct task {
// 	char* file_name;
// 	char status;
// } task;

queue* queue_create() {
	queue* q = malloc(sizeof(queue));
	q->head = NULL;
	q->tail = NULL;
	q->size = 0;
	return q;
}

void queue_destroy(queue* this) {
	queue_node* cur = this->head;
	while(cur != NULL) {
		queue_node* temp = cur;
		cur = cur->next;
		free_task(temp->data);
		free(temp);
	}
	free(this);
}

void queue_push(queue* this, task* element) {
	queue_node* node = malloc(sizeof(queue_node));
	node->next = NULL;
	node->data = element;
	if (this->head == NULL) {
		this->head = node;
		this->tail = node;
	} else {
		this->tail->next = node;
		this->tail = node;
	}
	this->size++;
}

task* queue_pull(queue* this) {

	if (this->size == 0)
		return NULL;
  	queue_node* node = this->head;

  	task* rt_data = node->data;
  	this->head = this->head->next;
  	if (this->head == NULL) {
    	this->tail = NULL;
  	}
  	this->size--;
  	free(node);
  	return rt_data;
}

int queue_empty(queue* this) {
	return (this->size == 0);
}

size_t queue_size(queue* this) {
	return this->size;
}
