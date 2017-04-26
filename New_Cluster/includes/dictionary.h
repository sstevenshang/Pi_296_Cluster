/**
 * Networking
 * CS 241 - Spring 2017
 */
#pragma once
#include "callbacks.h"
#include "compare.h"
#include "vector.h"
#include <stdbool.h>
#include <stdlib.h>
/**
 * In computer science, an associative array, map, symbol table, or dictionary
 * is
 * an abstract data type composed of a collection of (key, value) pairs, such
 * that
 * each possible key appears at most once in the collection.
 *
 * Operations associated with this data type allow:
 * the addition of a pair to the collection
 * the removal of a pair from the collection
 * the modification of an existing pair
 * the lookup of a value associated with a particular key
 *
 * https://en.wikipedia.org/wiki/Associative_array
 */

/* Forward declare dictionary structure. */
typedef struct dictionary dictionary;

struct key_value_pair {
  void *key;
  void **value;
};

typedef struct key_value_pair key_value_pair;

/**
 * Allocate and return a pointer to a new dictionary (on the heap).
 *
 * If you would like to make 'shallow' copies of the elements of the
 * 'dictionary',
 * then you may pass in NULL for the parameters (ex.
 * dictionary_create(hash_function, comp, NULL, NULL, NULL, NULL, NULL, NULL)).
 * This means that everytime an element is to be copied or removed from the
 * 'dictionary'
 * the pointer to that element is copied or removed
 * instead of using the user supplied copy constructor and destructor.
 *
 * If you supply NULL for the hash_function then all elements will hash to the
 * same index
 * causing all operations to require a linear scan of the dictionary.
 *
 * If you supply NULL for the compare then elements will be compared by their
 * virtual address.
 */
dictionary *dictionary_create(hash_function_type hash_function, compare comp,
                              copy_constructor_type key_copy_constructor,
                              destructor_type key_destructor,
                              copy_constructor_type value_copy_constructor,
                              destructor_type value_destructor);

/**
 * Destroys all container elements by
 * calling on the user provided destructor for every element,
 * and deallocates all the storage capacity allocated by the 'dictionary'.
*/
void dictionary_destroy(dictionary *this);

// Capacity:

/**
 * Returns whether 'this' is empty.
 */
bool dictionary_empty(dictionary *this);

/**
 * Returns the number of elements in 'this'.
 */
size_t dictionary_size(dictionary *this);

// Accessors and Modifiers

/**
 * Returns whether 'key' is present as a key of 'this'.
 */
bool dictionary_contains(dictionary *this, void *key);

/**
 * Returns the value of 'key' in 'this'.
 *
 * If 'key' does not exist in 'this', then NULL is returned.
 */
void *dictionary_get(dictionary *this, void *key);

/**
 * Returns a pointer to a key-value pair, which allows the user
 * to get or set the value directly.
 */
key_value_pair dictionary_at(dictionary *this, void *key);

/**
 * Adds the 'key'-'value' pair into 'this'.
 *
 * If 'key' already exist in 'this', then
 * the old value is destroyed and replaced with 'value'.
 */
void dictionary_set(dictionary *this, void *key, void *value);

/**
 * Removes 'key' and it's associated value from 'this'.
 *
 * If 'key' does not exist in 'this', then nothing happens.
 */
void dictionary_remove(dictionary *this, void *key);

/**
 * Destroys all elements of 'this'.
 */
void dictionary_clear(dictionary *this);

/**
 * Returns a pointer to a vector that contains all the keys of 'this'.
 */
vector *dictionary_keys(dictionary *this);

/**
 * Returns a pointer to a vector that contains all the values of 'this'.
 */
vector *dictionary_values(dictionary *this);
