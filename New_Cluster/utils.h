#pragma once

typedef void *(*copy_constructor_type)(void *elem);
typedef void (*destructor_type)(void *elem);
typedef void *(*default_constructor_type)(void);
