#pragma once

#define MASTER_HEARTBEAT_PORT (9110)
#define HEARTBEAT_SIZE (30)

typedef void *(*copy_constructor_type)(void *elem);
typedef void (*destructor_type)(void *elem);
typedef void *(*default_constructor_type)(void);
