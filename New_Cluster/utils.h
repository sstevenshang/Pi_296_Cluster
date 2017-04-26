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

#define START -1
#define DONE_SENDING -7
#define NOT_DONE_SENDING -8
#define NEED_SIZE -9
#define HAVE_SIZE -10
#define RECIEVING_DATA -11
#define FORWARD_DATA -12

#define WRONG_DATA_SIZE -16
#define COMMAND_BUF_SIZE 1024

typedef enum { INTERFACE_PUT, PUT} command;

typedef struct task {
  //Used for tracking files and book keeping
  int executable_fd;
  int output_fd;
  char* file_name;
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
  //Usage stat
  double CPU_usage;
  //Used for parsing and state tracking
  command to_do;
  int status;
  size_t file_size;
  int size_buffer_pos;
  char command[COMMAND_BUF_SIZE];
  int command_size;
  int temp_fd;
} worker;

ssize_t get_message_length_s(int socket, task* curr);

void respond_ok(int fd);

ssize_t transfer_fds(int fd1, int fd2, task* t);

int get_filename(int sfd, task* to_do);

int get_command(int sfd, task* to_do);

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
