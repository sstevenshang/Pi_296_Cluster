#pragma once

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdbool.h>
#include <pthread.h>
#include <errno.h>
#include <asm/atomic.h>

#include "queue.h"

#define PUT_REQUEST "PUT"
#define PUT_REQUEST_SIZE (strlen(PUT_REQUEST))
#define NUM_THREAD (5)
#define MAX_CONCURRENT_TASK (15)
#define OUTPUT_FILE_FORMAT "_OUTPUT.OUTPUT"
#define BAD_EXEC_MESSAGE "Bad executable: "
#define FILE_BUFFER_SIZE (4096)
#define LINE_BUFFER_SIZE (32)
#define MASTER_HEARTBEAT_PORT (9110);

typedef struct task_t{
    char* input_filename;
    char* output_filename;
    int output_fd;
} task_t;

void* task_copy_constructor(void* elem);
void task_destructor(void* elem);

int worker_main(char* host, char* port);
void* sending(void* nothing);
void* tasking(void* nothing);
int create_output_file(char* input_filename, char** output_filename);
void read_file(char* local_path);
char* get_filename_from_header(char* header, size_t header_size, size_t verb_size);
ssize_t write_size_to_socket(int socket_fd, size_t size);
ssize_t write_file_to_socket(int socket_fd, int local_file_fd, size_t local_file_size);
ssize_t write_to_socket(int socket_fd, char* buffer, ssize_t count);
ssize_t read_file_from_socket(int socket_fd, int local_file_fd, size_t count);
ssize_t read_from_socket(int socket_fd, char* buffer, ssize_t count);
ssize_t read_size_from_socket(int socket_fd, size_t* size);
ssize_t read_line_from_socket(int socket_fd, char** buffer);
void allocate_buffer(char** buffer, size_t size);
int setup_client(char* host, char* port);
char* create_header(char* filename);

void* heartbeat(void* master_address);
int send_heartbeat(int heartbeat_fd, struct sockaddr_in* master_info);
double get_local_usage();
int setUpUDPClient();
struct sockaddr_in setupUDPDestination(char* address);
void kill_heartbeat();
