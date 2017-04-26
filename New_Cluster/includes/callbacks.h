/**
 * Networking
 * CS 241 - Spring 2017
 */
#pragma once
#include <stdbool.h>
#include <stdlib.h>
/**
 * This header file contains all the callbacks used for our containers.
 */

/**
 * Declaration for a callback function that:
 *
 * Takes in a void pointer to arbitary data.
 * Allocates heap memory for said data.
 * Copies the values over from said data into the allocated heap memory.
 * Return a void pointer to the copy of said data on heap memory.
 *
 * Note that when a user adds an element to the container this function is used.
 */
typedef void *(*copy_constructor_type)(void *elem);

/**
 * Declaration for a callback function that:
 *
 * Takes in a void pointer to arbitary data on the heap.
 * Frees up the memory that data is taking up on the heap.
 *
 * Note that when a user removes an element from the container this function is
 * used.
*/
typedef void (*destructor_type)(void *elem);

/**
 * Declaration for a callback function that:
 *
 * Returns a pointer representing
 * the default value of the type of elements the container is storing.
 *
 * Note that whenever a default value of an object is needed this function is
 * used.
*/
typedef void *(*default_constructor_type)(void);

/**
 * Callback function the user should provide that takes in a pointer to
 * arbitrary data and returns a hash value to be used by the container.
 */
typedef size_t (*hash_function_type)(void *elem);

/**
 * The following are 'shallow' callback functions.
 * These are to be used when the user does not provide
 * their own callback functions for the container.
 */

void *shallow_copy_constructor(void *elem);

void shallow_destructor(void *elem);

void *shallow_default_constructor(void);

size_t shallow_hash_function(void *elem);

/**
 * char callback functions.
 */

void *char_copy_constructor(void *elem);

void char_destructor(void *elem);

void *char_default_constructor(void);

size_t char_hash_function(void *elem);

/**
 * double callback functions.
 */

void *double_copy_constructor(void *elem);

void double_destructor(void *elem);

void *double_default_constructor(void);

size_t double_hash_function(void *elem);

/**
 * float callback functions.
 */

void *float_copy_constructor(void *elem);

void float_destructor(void *elem);

void *float_default_constructor(void);

size_t float_hash_function(void *elem);

/**
 * int callback functions.
 */

void *int_copy_constructor(void *elem);

void int_destructor(void *elem);

void *int_default_constructor(void);

size_t int_hash_function(void *elem);

/**
 * long callback functions.
 */

void *long_copy_constructor(void *elem);

void long_destructor(void *elem);

void *long_default_constructor(void);

size_t long_hash_function(void *elem);

/**
 * short callback functions.
 */

void *short_copy_constructor(void *elem);

void short_destructor(void *elem);

void *short_default_constructor(void);

size_t short_hash_function(void *elem);

/**
 * unsigned char callback functions.
 */

void *unsigned_char_copy_constructor(void *elem);

void unsigned_char_destructor(void *elem);

void *unsigned_char_default_constructor(void);

size_t unsigned_char_hash_function(void *elem);

/**
 * unsigned int callback functions.
 */

void *unsigned_int_copy_constructor(void *elem);

void unsigned_int_destructor(void *elem);

void *unsigned_int_default_constructor(void);

size_t unsigned_int_hash_function(void *elem);

/**
 * unsigned long callback functions.
 */

void *unsigned_long_copy_constructor(void *elem);

void unsigned_long_destructor(void *elem);

void *unsigned_long_default_constructor(void);

size_t unsigned_long_hash_function(void *elem);

/**
 * unsigned short callback functions.
 */

void *unsigned_short_copy_constructor(void *elem);

void unsigned_short_destructor(void *elem);

void *unsigned_short_default_constructor(void);

size_t unsigned_short_hash_function(void *elem);
