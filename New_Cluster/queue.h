#pragma once
#include <stdlib.h>

typedef void *(*copy_constructor_type)(void *elem);
typedef void (*destructor_type)(void *elem);

typedef struct queue_node {
  void *data;
  struct queue_node *next;
} queue_node;

typedef struct queue {
  copy_constructor_type copy_constructor;
  destructor_type destructor;
  queue_node *head, *tail;
  ssize_t size;
  ssize_t max_size;
  pthread_cond_t cv;
  pthread_mutex_t m;
} queue;

queue* queue_create (ssize_t max_size, copy_constructor_type copy_constructor, destructor_type destructor);
void queue_destroy (queue* this);
void queue_push (queue* this, void* element);
void* queue_pull (queue* this);
