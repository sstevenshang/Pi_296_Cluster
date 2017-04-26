#include "vector.h"

#define INITIAL_CAPACITY 10
#define GROWTH_FACTOR 2

static size_t get_new_capacity(size_t target) {
  size_t new_capacity = 1;
  while (new_capacity < target) {
    new_capacity *= GROWTH_FACTOR;
  }
  return new_capacity;
}

vector *vector_create(copy_constructor_type copy_constructor, destructor_type destructor, default_constructor_type default_constructor) {
    vector * result = malloc(sizeof(vector));
    result->array = malloc(INITIAL_CAPACITY * sizeof(void*));
    result->size = 0;
    result->capacity = INITIAL_CAPACITY;
    for (size_t i=0; i < result->capacity; i++) {
        result->array[i] = 0;
    }

    result->copy_constructor = (!copy_constructor) ? (shallow_copy_constructor) : (copy_constructor);
    result->destructor = (!destructor) ? (shallow_destructor) : (destructor);
    result->default_constructor = (!default_constructor) ? (shallow_default_constructor) : (default_constructor);
    return result;
}

void vector_destroy(vector *this) {
    for (size_t i=0; i < this->size; i++) {
        this->destructor(this->array[i]);
    }
    free(this->array);
    this->size = 0;
    this->capacity = 0;
    free(this);
}

void **vector_begin(vector *this) {
    return this->array + 0;
}

void **vector_end(vector *this) {
    return this->array + this->size;
}

size_t vector_size(vector *this) {
    return this->size;
}

void vector_resize(vector *this, size_t n) {
    if (n == this->size) return;
    else if (n < this->size) {
        for (size_t i = this->size - 1; i > n-1; i--) {
            this->destructor(this->array[i]);
            this->array[i] = 0;
        }
    }
    else {
        if (n > this->capacity) {
            vector_reserve(this, n);
        }
        for (size_t i = this->size; i < n; i++) {
            this->array[i] = this->default_constructor();
        }
    }
    this->size = n;
}

size_t vector_capacity(vector *this) {
    return this->capacity;
}

bool vector_empty(vector *this) {
    return !(this->size);
}

void vector_reserve(vector *this, size_t n) {
    if (n > this->capacity) {
        size_t newcap = get_new_capacity(n);
        this->array = realloc(this->array, newcap * sizeof(void*));
        for (size_t i=this->capacity; i<newcap; i++) {
            this->array[i] = 0;
        }
        this->capacity = newcap;
    }
}

void **vector_at(vector *this, size_t position) {
    return &(this->array[position]);
}

void vector_set(vector *this, size_t position, void *element) {
    this->destructor(this->array[position]);
    this->array[position] = this->copy_constructor(element);
}

void *vector_get(vector *this, size_t position) {
    return this->array[position];
}

void **vector_front(vector *this) {
    return this->array + 0;
}

void **vector_back(vector *this) {
    return this->array + this->size - 1;
}

void vector_push_back(vector *this, void *element) {
    size_t newsize = this->size + 1;
    if (newsize > this->capacity)
        vector_reserve(this, newsize);
    void ** end = vector_end(this);
    *end = this->copy_constructor(element);
    this->size++;
}

void vector_pop_back(vector *this) {
    if (vector_empty(this)) return;
    this->destructor(this->array[this->size - 1]);
    this->array[this->size - 1] = 0;
    this->size--;
}

void vector_insert(vector *this, size_t position, void *element) {
    size_t newsize = this->size + 1;
    if (newsize > this->capacity)
        vector_reserve(this, newsize);
    if (position < this->size) {
        void * src = this->array + position;
        void * dest = this->array + position + 1;
        memmove(dest, src, (this->size - position)*sizeof(void*));
    }
    this->array[position] = this->copy_constructor(element);
    this->size++;
}

void vector_erase(vector *this, size_t position) {
    if (position == this->size - 1) {
        vector_pop_back(this);
        return;
    }
    this->destructor(this->array[position]);
    this->array[position] = 0;
    void * src = this->array + position + 1;
    void * dest = this->array + position;
    memmove(dest, src, (this->size - position - 1)*sizeof(void*));
    this->array[this->size-1] = 0;
    this->size--;
}

void vector_clear(vector *this) {
    for (size_t i=0; i<this->size; i++) {
        this->destructor(this->array[i]);
        this->array[i] = 0;
    }
    this->size = 0;
}
