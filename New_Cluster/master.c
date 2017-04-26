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

#include "vector.h"

static int server_socket;
static int close_master = 0;
static int epoll_fd;
static char* temp_directory;

static int interface_fd = -1;
static worker* interface = NULL;
static vector* worker_list;


#define START -1
#define DONE_SENDING -7
#define NOT_DONE_SENDING -8
#define NEED_SIZE -9
#define HAVE_SIZE -10
#define RECIEVING_DATA -11
#define FORWARD_DATA -12

#define WRONG_DATA_SIZE -16
#define COMMAND_BUF_SIZE 1024

/*
    BELOW CODE ARE FROM UTILS.H
    SINCE ONLY USED BY MASTER.C, MOVE HERE
    TO INCREASE CODE CLARITY
*/

typedef enum { INTERFACE_PUT, PUT} command;

typedef struct task {
  int executable_fd;
  int output_fd;
  char* file_name;
} task;

typedef struct worker {
  int worker_fd; //Socket connection with a worker
  int alive; //Toggle for "alive" status of worker
  char* IP; //IP address of the worker
  vector* tasks; //Vector of tasks that the worker is working on
  double CPU_usage; //Usage stat
  command to_do; //Used for parsing and state tracking
  int status;
  size_t file_size;
  int size_buffer_pos;
  char command[COMMAND_BUF_SIZE];
  int command_size;
  int temp_fd;
  char* temp_file_name;
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

/*
    ABOVE CODE ARE FROM UTILS.H
    SINCE ONLY USED BY MASTER.C, MOVE HERE
    TO INCREASE CODE CLARITY
*/

void kill_master() { close_master = 1; }

void ignore() { }

void setSignalHandlers() {
  struct sigaction act;
  memset(&act, '\0', sizeof(act));
  act.sa_handler = kill_master;
  if (sigaction(SIGINT, &act, NULL) < 0) {
    perror("sigaction");
    exit(1);
  }
  struct sigaction act2;
  memset(&act2, '\0', sizeof(act2));
  act2.sa_handler = ignore;
  if (sigaction(SIGPIPE, &act2, NULL) < 0) {
    perror("sigaction");
    exit(1);
  }
}

void cleanGlobals() {
  //Cleanup directory
  chdir("..");
  free(temp_directory);
  //Close file descriptors
  close(epoll_fd);
  close(server_socket);
}

void setUpGlobals(char* port) {
  server_socket = set_up_server(port);
	epoll_fd = epoll_create(1);
  worker_list = vector_create(NULL, NULL, NULL);

	if(epoll_fd == -1) {
    clean_up_globals();
    perror("epoll_create()");
    exit(1);
  }

	struct epoll_event event;
	event.data.fd = server_socket;
	event.events = EPOLLIN | EPOLLET;
	if(epoll_ctl(epoll_fd, EPOLL_CTL_ADD, server_socket, &event)) {
    clean_up_globals();
    perror("epoll_ctl()");
    exit(1);
	}

  char dummy[] = "./master_tempXXXXXX";
  temp_directory = strdup(mkdtemp(dummy));
  print_temp_directory(temp_directory);
  chdir(temp_directory);
}

void reset_worker_for_parsing(worker* newWorker) {
  newWorker->temp_fd = -1;
  for (int i = 0; i < COMMAND_BUF_SIZE; i++)
    newWorker->command[i] = 0;
  newWorker->command_size = 0;
  newWorker->size_buffer_pos = 0;
  newWorker->status = START;
  newWorker->temp_fd = -1;
  newWorker->file_size = 0;
  if (newWorker->temp_file_name)
    free(newWorker->temp_file_name)
  newWorker->temp_file_name = NULL;
}

worker* create_worker(int fd, char* IP){
  worker* newWorker = (worker*)malloc(sizeof(worker));
  newWorker->alive = 1;
  newWorker->tasks = vector_create(NULL, NULL, NULL);
  newWorker->worker_fd = fd;
  newWorker->IP = strdup(IP);
  newWorker->temp_file_name = NULL;
  reset_worker_for_parsing(newWorker);
  return newWorker;
}

//Make sure you shutdown + close worker_fd before calling.
void free_worker(worker* to_free) {
  vector_destroy(to_free->tasks);
  free(to_free->IP);
  free(to_free);
  to_free = NULL;
}

ssize_t find_worker_pos(int fd){
  size_t i = 0;
  while(i < vector_size(worker_list)){
    if((vector_get(worker_list, i))->worker_fd == fd){
      return i;
    }
    i++
  }
  return -1;
}

void accept_connections(struct epoll_event *e,int epoll_fd) {
	while(1) {
		struct sockaddr_in new_addr;
		socklen_t new_len = sizeof(new_addr);
		int new_fd = accept(e->data.fd, (struct sockaddr*) &new_addr, &new_len);

		if(new_fd == -1)
		{
			// All pending connections handled
			if(errno == EAGAIN || errno == EWOULDBLOCK)
				break;
			else {
				perror("accept");
				exit(1);
			}
		}
    char *connected_ip= inet_ntoa(new_addr.sin_addr);

    int flags = fcntl(new_fd, F_GETFL, 0);
    fcntl(new_fd, F_SETFL, flags | O_NONBLOCK);

    //Set up worker struct. Add to list
    worker* newWorker = create_worker(new_fd, connected_ip);
    vector_push_back(worker_list, newWorker);

    //Connection to epoll
    struct epoll_event event;
    event.data.fd = new_fd;
    event.events = EPOLLIN | EPOLLET;
    if(epoll_ctl(epoll_fd, EPOLL_CTL_ADD, new_fd, &event) == -1) {
      perror("accept epoll_ctl");
      exit(1);
    }
	}
}

void handle_data(struct epoll_event *e) {
    int vector_pos = find_worker_pos(e->data.fd);
    worker* curr;
    ssize_t check;

    if (vector_pos != -1)
      curr = vector_get(worker_list, vector_pos);
    else if (vector_pos == -1 || )
      curr = interface;

    if(interface_fd == -1){
	    interface_fd = curr->worker_fd;
	    interface = curr;
	    vector_erase(worker_list,find_worker_pos(e->data.fd));
    }

    if (curr->status == START) {
      check = get_command(e->data.fd, curr);
      switch (check) {
        case NOT_DONE_SENDING:
          return;
        case DONE_SENDING:
          curr->state = NEED_SIZE;
          break;
        default:
          fprintf(stderr, "There was a catastrophic failure!\n");
      }
    }
    if (curr->status == NEED_SIZE) {
      check = get_size(e->data.fd, curr);
      switch (check) {
        case NOT_DONE_SENDING:
          return;
        case DONE_SENDING:
          curr->state = RECIEVING_DATA;
          break;
        default:
          fprintf(stderr, "There was a catastrophic failure!\n");
      }
    }
    if (curr->status == RECIEVING_DATA) {
      check = get_binary_data(e->data.fd, curr);
      switch (check) {
        case NOT_DONE_SENDING:
          return;
        case DONE_SENDING:
          curr->state = FORWARD_DATA;
          break;
        default:
          fprintf(stderr, "There was a catastrophic failure!\n");
      }
    }
    if (curr->status == FORWARD_DATA) {


    }


}

int main(int argc, char** argv) {

  if (argc != 2) {
    printf("Usage : ./master <port>\n");
    exit(1);
  }

  setSignalHandlers();
  setUpGlobals(argv[1]);

	// Event loop
	while(!close_master) {

		struct epoll_event new_event;

		if(epoll_wait(epoll_fd, &new_event, 1, -1) > 0)
		{
			if(server_socket == new_event.data.fd)
				accept_connections(&new_event, epoll_fd);
			else
				handle_data(&new_event);
		}
	}

  cleanGlobals();

  return 0;
}







/*
    BELOW CODE ARE FROM UTILS.H
    SINCE ONLY USED BY MASTER.C, MOVE HERE
    TO INCREASE CODE CLARITY
*/

void reset_header_buffer(task* curr) {
  for (int i = 0; i < HEADER_BUFFER_SIZE; i++)
    curr->header[i] = 0;
}

void respond_ok(int fd) {
  write_all_to_socket(fd, "OK\n", 3);
}

ssize_t get_message_length_s(int socket, task* curr) {
  int rd = -1;
  while (rd == -1) {
      rd = read(socket, ((char*) &curr->file_size) + curr->size_buffer_pos, sizeof(size_t)-curr->size_buffer_pos);
      if (rd == -1)
        if (errno != EINTR)
          return NOT_DONE_SENDING;
      if (rd == 0) {
        return INVALID_COMMAND;
      }
      if (rd != -1)
        break;
  }
  if (rd > 0) {
    curr->size_buffer_pos += rd;
    if (curr->size_buffer_pos == sizeof(size_t))
      return DONE_SENDING;
    return NOT_DONE_SENDING;
  } else if (rd == 0) {
    return INVALID_COMMAND;
  }
  return NOT_DONE_SENDING;
}

ssize_t transfer_fds(int socket, int fd, task* t) {
  char buffer[4096];
  ssize_t curr_read;
  ssize_t total_read = 0;
  errno = 0;
  while (1) {
    curr_read = read(socket, buffer, 4096);
    if (curr_read == -1) {
      if (errno == EINTR)
        continue;
      else if (errno == EWOULDBLOCK || errno == EAGAIN || errno == 11){
        return NOT_DONE_SENDING;
      }
      return -1;
    } else if (curr_read == 0) {
        break;
    }
    ssize_t written = write(fd, buffer, curr_read);
    t->file_size -= curr_read;
    total_read += curr_read;

    if (written == - 1 && errno == EPIPE)
      return -1;
  }
  if (t->file_size != 0 || read(socket, buffer, 1) > 0) {
    return WRONG_DATA_SIZE;
  }
  return DONE_SENDING;
}

//return 1 on success, 0 on failure
int get_filename(int sfd, task* to_do) {
  char b;
  int idx = 0;
  ssize_t read;

  for (;idx < HEADER_BUFFER_SIZE; idx++)
    if (!idx)
      break;

  while ( (read = read_all_from_socket(sfd, &b, 1)) != -1 && b != '\n' && read != 0) {
    to_do->head_size++;
    if (to_do->head_size > 1024) {
      return INVALID_COMMAND;
    }
    if (b != ' ')
      to_do->header[idx++] = b;
  }
  if (b == '\n'){
    if (to_do->head_size > 1024) {
      return INVALID_COMMAND;
    }
    return 0;
  }
  if (!read) {
    return INVALID_COMMAND;
  }
  return 1;
}

int get_command(task* to_do) {
      if (strncmp(to_do->header, "INTERFACE_PUT", 13) == 0) {
        return 1;
      } else if (strncmp(to_do->header, "PUT", 3) == 0) {
        return 2;
      }
  return -1;
}

task set_up_blank_task() {
  task t = (task) {.request=NULL, .to_do=GET, .status=GETTING_VERB,.file_size=0,.size_buffer_pos=0,.head_size=0};
  for (int i = 0; i < HEADER_BUFFER_SIZE; i++) {
    t.header[i] = 0;
  }
  return t;
}

void *string_copy_constructor(void *elem) {
  char *str = elem;
  return strdup(str);
}

void string_destructor(void *elem) { free(elem); }

void *string_default_constructor(void) {
  return calloc(1, sizeof(char));
}

vector* string_vector_create() {
  return vector_create(string_copy_constructor, string_destructor, string_default_constructor);
}

void* task_copy_constructor(void* elem) {
  return new_task;
}

void task_destructor(void* elem) {
  free(elem);
}

int set_up_server(char* port) {
  int s;
  int sock_fd = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0);
  struct addrinfo hints, *result = NULL;
  memset(&hints, 0, sizeof(struct addrinfo));
  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = AI_PASSIVE;
  s = getaddrinfo(NULL, port, &hints, &result);
  if (s != 0) {
    fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(s));
    exit(1);
  }
  int optval = 1;
  setsockopt(sock_fd, SOL_SOCKET, SO_REUSEPORT, &optval, sizeof(optval));
  if (bind(sock_fd, result->ai_addr, result->ai_addrlen) != 0 ) {
        perror("bind()");
        exit(1);
  }
  if (listen(sock_fd, 10) != 0 ) {
    perror("listen()");
    exit(1);
  }
  return sock_fd;
}

int connect_to_server(const char *host, const char *port) {
    int socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    struct addrinfo hints, *result;
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_INET; //IPv4 Only
    hints.ai_socktype = SOCK_STREAM; //TCP
    int addr_info_error = getaddrinfo(host, port, &hints, &result);
    if (addr_info_error) {
      fprintf(stderr, "%s\n", gai_strerror(addr_info_error));
      return -1;
    }
    if (connect(socket_fd, result->ai_addr, result->ai_addrlen)) {
      perror(NULL);
      freeaddrinfo(result);
      return -1;
    }
    freeaddrinfo(result);
    return socket_fd;
}

ssize_t write_all_to_socket(int socket, const char *buffer, size_t count) {
    ssize_t curr_write;
    ssize_t total_write = 0;
    errno = 0;

    while(count) {
      curr_write = write(socket, buffer, count);
      if (curr_write == -1) {
        if (errno == EINTR)
          continue;
        return -1;
      } else if (curr_write == 0)
        break;
      count -= curr_write;
      total_write += curr_write;
      buffer += curr_write;
    }
    return total_write;
}

void shutdown_further_writes(int socket) {
  shutdown(socket, SHUT_WR);
}

void shutdown_further_reads(int socket) {
  shutdown(socket, SHUT_RD);
}

ssize_t read_all_from_socket(int socket, char *buffer, size_t count) {
    ssize_t curr_read;
    ssize_t total_read = 0;
    errno = 0;
    while(count) {
      curr_read = read(socket, buffer, count);
      if (curr_read == -1) {
        if (errno == EINTR)
          continue;
        return -1;
      } else if (curr_read == 0)
        break;
      count -= curr_read;
      total_read += curr_read;
      buffer += curr_read;
    }
    return total_read;
}

size_t get_file_size(char* file_name) {
  FILE* fp = fopen(file_name, "r");

  if (!fp) {
    return 0;
  }

  rewind(fp);
  fseek(fp, 0L, SEEK_END);
  size_t size = ftell(fp);
  rewind(fp);
  fclose(fp);
  return size;
}

/*
    ABOVE CODE ARE FROM UTILS.H
    SINCE ONLY USED BY MASTER.C, MOVE HERE
    TO INCREASE CODE CLARITY
*/
