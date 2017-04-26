#pragma once
#include <errno.h>
#include <fcntl.h>
#include <netdb.h>
#include <signal.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include "vector.h"

#define HEADER_BUFFER_SIZE 4096
#define RECIEVING_HEADER 1
#define RECEIVED_HEADER 2
#define RECIEVING_DATA 3
#define DONE_SENDING -7
#define NOT_DONE_SENDING -8
#define INVALID_COMMAND -11
#define GETTING_VERB -12
#define HAVE_VERB -13
#define HAVE_FILENAME -14
#define HAVE_SIZE -15
#define WRONG_DATA_SIZE -16
#define SEND_RESPONSE -18

typedef enum { INTERFACE_PUT, PUT} verb;

typedef struct task {
  //Used for tracking files and book keeping
  int executable_fd;
  int output_fd;
  char* file_name;

  //Used for parsing and state tracking
  verb to_do;
  int status;
  size_t file_size;
  int size_buffer_pos;
  char header[HEADER_BUFFER_SIZE];
  int head_size;
} task;

typedef struct worker {
  //Socket connection with a worker
  int worker_fd;
  //Toggle for "alive" status of worker
  int alive;
  //IP address of the worker
  char* IP;
  //Vector of tasks that the worker is working on
  vector* tasks;
  int status;
} worker;

ssize_t get_message_length_s(int socket, task* curr);

void respond_ok(int fd);

ssize_t transfer_fds(int fd1, int fd2, task* t);

int get_filename(int sfd, task* to_do);

int get_verb(int sfd, task* to_do);

task set_up_blank_task();

vector* string_vector_create();

void *string_default_constructor(void);

void string_destructor(void *elem);

void* task_copy_constructor(void* elem);

void task_destructor(void* elem);

int set_up_server(char* port);

int connect_to_server(const char *host, const char *port);

ssize_t write_all_to_socket(int socket, const char *buffer, size_t count);

void shutdown_further_writes(int socket);

void shutdown_further_reads(int socket);

ssize_t read_all_from_socket(int socket, char *buffer, size_t count);

size_t get_file_size(char* file_name);
