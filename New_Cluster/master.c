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

static int sock_fd;
static int close_master = 0;
static int epoll_fd;
static char* temp_directory;

static int interface_fd = -1;
static vector* worker_list;

void close_server() { close_master = 1; }

void ignore() { }

void set_up_signals() {
  struct sigaction act;
  memset(&act, '\0', sizeof(act));
  act.sa_handler = close_server;
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

void set_up_worker_list(){
  worker_list = vector_create(NULL, NULL, NULL);
}

void create_worker(int fd){
  worker* newWorker = (worker*)malloc(sizeof(worker));
  newWorker->alive = 1;
  newWorker->tasks = vector_create(NULL, NULL, NULL);
  newWorker->worker_fd = fd;
  newWorker->status = -1;
  return newWorker;
}

size_t find_worker_pos(int fd){

  size_t i = 0;
  iwhile(i < vector_size(worker_list)){
    if((vector_get(worker_list, i))->worker_fd == fd){
      return i;
    }
    i++
  }
  return -1;

}

void clean_up_globals() {
  //Cleanup directory
  for (unsigned i = 0; i < vector_size(files); i++)
    remove((char*) vector_get(files, i));
  chdir("..");
  // printf("Removing %s\n", temp_directory);
  rmdir(temp_directory);
  free(temp_directory);
  //Close file descriptors
  close(epoll_fd);
  close(sock_fd);
  vector_destroy(files);
}

void set_up_gloabls(char* port) {
  sock_fd = set_up_server(port);
	epoll_fd = epoll_create(1);
	if(epoll_fd == -1) {
    clean_up_globals();
    perror("epoll_create()");
    exit(1);
  }
	struct epoll_event event;
	event.data.fd = sock_fd;
	event.events = EPOLLIN | EPOLLET;
	if(epoll_ctl(epoll_fd, EPOLL_CTL_ADD, sock_fd, &event)) {
    clean_up_globals();
    perror("epoll_ctl()");
    exit(1);
	}
  char dummy[] = "./master_tempXXXXXX";
  temp_directory = strdup(mkdtemp(dummy));
  print_temp_directory(temp_directory);
  chdir(temp_directory);
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

    int flags = fcntl(new_fd, F_GETFL, 0);
    fcntl(new_fd, F_SETFL, flags | O_NONBLOCK);

    //TODO set up worker struct. Add to list

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

void handle_data(struct epoll_event *e)
{
    //task* curr = dictionary_get(dick, &e->data.fd);
    worker* curr = vector_get(worker_list,find_worker_pos(e->data.fd));
          if(interface_fd == -1){
	    interface_fd = curr->worker_fd;
	    interface = curr;
	    vector_remove(worker_list,find_worker_pos(e->data.fd));
          }
	  if(interface_fd == curr-> worker_fd){
	    schedule();
	//TODO
   	  }
	  int type = get_verb(curr);
   	  if(type == -1 || type == 1){return ;}//bad verb (1 shoudl be handled by scheduler

}

int main(int argc, char** argv) {

  if (argc != 2) {
    printf("Usage : ./master <port>\n");
    exit(1);
  }

  set_up_signals();

  set_up_gloabls(argv[1]);

	// Event loop
	while(!close_master) {

		struct epoll_event new_event;

		if(epoll_wait(epoll_fd, &new_event, 1, -1) > 0)
		{
			if(sock_fd == new_event.data.fd)
				accept_connections(&new_event, epoll_fd);
			else
				handle_data(&new_event);
		}
	}

  clean_up_globals();

  return 0;
}
