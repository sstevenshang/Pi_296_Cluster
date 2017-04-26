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
#include "utils.h"
#include "callbacks.h"
#include "vector.h"

static int server_socket;
static int close_master = 0;
static int epoll_fd;
static char* temp_directory;

static int interface_fd = -1;
static worker* interface = NULL;
static vector* worker_list;

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

worker* create_worker(int fd, char* IP){
  worker* newWorker = (worker*)malloc(sizeof(worker));
  newWorker->alive = 1;
  newWorker->tasks = vector_create(NULL, NULL, NULL);
  newWorker->worker_fd = fd;
  newWorker->status = START;
  newWorker->IP = strdup(IP);
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
    else
      curr = interface;

    if(interface_fd == -1){
	    interface_fd = curr->worker_fd;
	    interface = curr;
	    vector_erase(worker_list,find_worker_pos(e->data.fd));
    }

    if (curr->status == START) {
      check = parse_command(e->data.fd, curr);
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
