#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/epoll.h>
#include <errno.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <signal.h>
#include <stddef.h>
#include <time.h>
#include <pthread.h>

#include "vector.h"
#include "utils.h"

#define START -1
#define DONE_SENDING -7
#define NOT_DONE_SENDING -8
#define NEED_SIZE -9
#define HAVE_SIZE -10
#define RECIEVING_DATA -11
#define FORWARD_DATA -12
#define FORWARDING_DATA -13
#define BIG_FAILURE -3

#define WRONG_DATA_SIZE -16
#define COMMAND_BUF_SIZE 1024

typedef enum { INTERFACE_PUT, PUT} command;

typedef struct task {
  char* file_name;
} task;

typedef struct worker {
  int worker_fd; //Socket connection with a worker
  int alive; //Toggle for "alive" status of worker
  char* IP; //IP address of the worker
  vector* tasks; //Vector of tasks that the worker is working on
  double CPU_usage; //Usage stat
  double last_beat_received; //For heartbeat

  //Used for parsing and state tracking
  int status;

  char command[COMMAND_BUF_SIZE];
  int command_size;

  command to_do;
  char* temp_file_name;

  size_t file_size;
  int size_buffer_pos;

  int temp_fd;
  int fd_to_send_to;
  size_t file_buffer_pos;
} worker;

/*
  Schedule returns the fd of the worker to send the task to while interally
  updating it's vector of current tasks. Doesn't manipulate any files. Used for
  bookeeping purposes to tell what tasks are allocated to what nodes as well as
  telling the main event loop how to allocate work.

  Scheduler remove task removes a given task from a given worker's list of
  tasks (implicitly indicating the task as satisfied). Doesn't manipulate any
  files. Simply used for bookeeping purposes to take a task off of a worker's
  task list.
*/
int schedule(task* t, vector* worker_list);
void scheduler_remove_task(int worker_fd, char* filename, vector* worker_list);

/*
  Helper functions central to master's functionality.
*/
ssize_t get_command(worker* to_do);
ssize_t get_size(worker* curr);
int open_with_all_permission(char* filename);
ssize_t get_binary_data(worker* curr);
task* make_task(worker* w);

ssize_t do_put(int fd_to_send_to, worker* w);

int set_up_server(char* port);
ssize_t write_all_to_socket(int socket, const char *buffer, size_t count);
void shutdown_further_writes(int socket);
void shutdown_further_reads(int socket);
ssize_t read_all_from_socket(int socket, char *buffer, size_t count);
size_t get_file_size(char* file_name);

void kill_master();
void ignore();
void setSignalHandlers();
void cleanGlobals();
void setUpGlobals(char* port);
void reset_worker_for_parsing(worker* newWorker);
worker* create_worker(int fd, char* IP);
void free_worker(worker* to_free);
ssize_t find_worker_pos(int fd);
void accept_connections(struct epoll_event *e,int epoll_fd);
void handle_data(struct epoll_event *e);
int master_main();
void checkOnNodes();

void* listen_to_heartbeat(void* nothing);
void report_heartbeat(char* worker_addr, double client_usage);
void* detect_heart_failure(void* nothing);
int setUpUDPServer();
double getTime();

void reschedule(worker* dead_worker);
