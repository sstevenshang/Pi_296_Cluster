#pragma once
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include "utils.h"

typedef struct vector {
  copy_constructor_type copy_constructor;
  destructor_type destructor;
  default_constructor_type default_constructor;
  void **array;
  size_t size;
  size_t capacity;
} vector;

vector *vector_create(copy_constructor_type copy_constructor, destructor_type destructor, default_constructor_type default_constructor);
void vector_destroy(vector *this);
void **vector_begin(vector *this);
void **vector_end(vector *this);
size_t vector_size(vector *this);
void vector_resize(vector *this, size_t n);
size_t vector_capacity(vector *this);
bool vector_empty(vector *this);
void vector_reserve(vector *this, size_t n);
void **vector_at(vector *this, size_t n);
void vector_set(vector *this, size_t n, void *element);
void *vector_get(vector *this, size_t n);
void **vector_front(vector *this);
void **vector_back(vector *this);
void vector_push_back(vector *this, void *element);
void vector_pop_back(vector *this);
void vector_insert(vector *this, size_t position, void *element);
void vector_erase(vector *this, size_t position);
void vector_clear(vector *this);

void *shallow_copy_constructor(void *elem);
void shallow_destructor(void *elem);
void *shallow_default_constructor(void);
