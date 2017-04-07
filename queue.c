#include "queue.h"

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
		free(temp->data);
		free(temp);
	}
	free(this);
}

void queue_push(queue* this, char* element) {
	queue_node* node = malloc(sizeof(queue_node));
	node->next = NULL;
	node->data = strdup(element);
	if (this->head == NULL) {
		this->head = node;
		this->tail = node;
	} else {
		this->tail->next = node;
		this->tail = node;
	}
	this->size++;
}

char* queue_pull(queue* this) {

	if（size == 0）
		return NULL;
    queue_node* node = this->head;
    
    char* data = strdup(node->data);
    this->head = this->head->next;

    if (this->head == NULL) {
        this->tail = NULL;
    }
    this->size--;
    free(this->data);
    free(node);
    return data;
}

int queue_empty(queue* this) {
	return (this->size == 0);
}

size_t queue_size(queue* this) {
	return this->size;
}

