#include "queue.h"
#include <pthread.h>

queue *queue_create(ssize_t max_size, copy_constructor_type copy_constructor, destructor_type destructor) {
    queue* q = malloc(sizeof(queue));
    q->max_size = max_size;
    q->copy_constructor = copy_constructor;
    q->destructor = destructor;
    q->head = NULL;
    q->tail = NULL;
    q->size = 0;
    pthread_mutex_init(&(q->m), NULL);
    pthread_cond_init(&(q->cv), NULL);
    return q;
}

void queue_destroy(queue *this) {
    pthread_mutex_destroy(&(this->m));
    pthread_cond_destroy(&(this->cv));
    queue_node* cur = this->head;
    while(cur != NULL) {
        queue_node* temp = cur;
        cur = cur->next;
        this->destructor(temp->data);
        free(temp);
    }
    free(this);
}

void queue_push(queue *this, void *data) {
    pthread_mutex_lock(&(this->m));
    while (this->max_size > 0 && this->size == this->max_size) {
        pthread_cond_wait(&(this->cv), &(this->m));
    }
    queue_node* node = malloc(sizeof(queue_node));
    node->next = NULL;
    node->data = this->copy_constructor(data);
    if (this->head == NULL) {
        this->head = node;
        this->tail = node;
    } else {
        this->tail->next = node;
        this->tail = node;
    }
    this->size++;
    if (this->size == 1) {
        pthread_cond_broadcast(&(this->cv));
    }
    pthread_mutex_unlock(&(this->m));
}

void *queue_pull(queue *this) {
    pthread_mutex_lock(&(this->m));
    while (this->size == 0) {
        pthread_cond_wait(&(this->cv), &(this->m));
    }
    queue_node* node = this->head;
    void* data = node->data;
    this->head = this->head->next;
    if (this->head == NULL) {
        this->tail = NULL;
    }
    this->size--;
    free(node);
    if (this->max_size > 0 && this->size == (this->max_size-1)) {
        pthread_cond_broadcast(&(this->cv));
    }
    pthread_mutex_unlock(&(this->m));
    return data;
}
