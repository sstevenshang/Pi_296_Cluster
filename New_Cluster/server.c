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
#include "common.h"
#include "callbacks.h"
#include "vector.h"

static int sock_fd;
static int endSession = 0;
static int epoll_fd;
static char* temp_directory;
static ssize_t ret;
static int interface_fd = -1;
vector* worker_list;
void close_server() { endSession = 1; }

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
  files = string_vector_create();

	//setup epoll
	epoll_fd = epoll_create(1);
	if(epoll_fd == -1) {
    clean_up_globals();
    perror("epoll_create()");
    exit(1);
  }

	struct epoll_event event;
	event.data.fd = sock_fd;
	event.events = EPOLLIN | EPOLLET;

	//Add the sever socket to the epoll
	if(epoll_ctl(epoll_fd, EPOLL_CTL_ADD, sock_fd, &event)) {
    clean_up_globals();
    perror("epoll_ctl()");
    exit(1);
	}
  char dummy[] = "./storageXXXXXX";
  temp_directory = strdup(mkdtemp(dummy));
  print_temp_directory(temp_directory);
  chdir(temp_directory);
}

void accept_connections(struct epoll_event *e,int epoll_fd)
{
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

		// char *connected_ip= inet_ntoa(new_addr.sin_addr);
		// int port = ntohs(new_addr.sin_port);
    // printf("Accepted Connection %s port %d\n", connected_ip, port);

    int flags = fcntl(new_fd, F_GETFL, 0);
    fcntl(new_fd, F_SETFL, flags | O_NONBLOCK);

    // printf("connection on fd %i\n", new_fd);
    worker* tmpWorker = create_worker(new_fd);
    vector_push(worker_list,tmpWorker);
    //Add to our dictionary
    task to_do = (task) set_up_blank_task();
    // to_do.request = fopen("request", "a+");
    //dictionary_set(dick, &new_fd, &to_do);

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
	    vector_remove(worker_list,find_worker_pos(e->data.fd));
          }
	  if(interface_fd == curr-> worker_fd){
	    schedule();
	//TODO
   	  }
          ret = get_filename(e->data.fd, curr);
          // printf("ret is %zi\n", ret);
          if (ret && ret != INVALID_COMMAND){
            return;
          } else if (ret == INVALID_COMMAND) {
            //TODO send error message
            respond_bad_request(e->data.fd);
            discard_task(e->data.fd);
            return;
          }
          // printf("GOT FILENAME\n" );
          // printf("Filename is %s\n", curr->header);
          if (strlen(curr->header) == 0) {
            respond_bad_request(e->data.fd);
            discard_task(e->data.fd);
          }
          curr->status = HAVE_FILENAME;
          break;
        case LIST:
          // printf("DO LIST!\n");
          ret = send_file_list(e->data.fd, files);
          if (ret == -1) {
            return;
          } else if (ret == NOT_DONE_SENDING)
            break;
          return;
          break;
      }
    }

    if (curr->status == HAVE_FILENAME) {
      switch (curr->to_do) {
        case PUT:
          // printf("Do PUT_get filename size!\n");
          ret = get_message_length_s(e->data.fd, curr);
          if (ret == DONE_SENDING) {
            curr->status = HAVE_SIZE;
            // printf("GOT SIZE: %zi\n", curr->file_size);
          } else if (ret == NOT_DONE_SENDING) {
            return;
          } else if (ret == INVALID_COMMAND) {
            //TODO send error message, remove from dictionary, discard of file descriptor
            return;
          }
          break;
        case GET:
          printf("DO GET!\n");
          return;
          break;
        case DELETE:
          printf("DO DELETE!\n");
          return;
          break;
        default:
          break;
      }
    }
    if ((curr->status == HAVE_SIZE || curr->status == RECIEVING_DATA) && curr->to_do == PUT) {
      // printf("Next is to recieve the file\n");
      if (curr->status == HAVE_SIZE) {
        vector_push_back(files, curr->header);
        curr->status = RECIEVING_DATA;
      }
      if (!curr->request)
        curr->request = fopen(curr->header, "wb+");
      ssize_t written = transfer_fds(e->data.fd, fileno(curr->request), curr);
      if (written == -1) {
        //TODO send error message
        return;
      } else if (written == TOO_MUCH_DATA || written == TOO_LITTLE_DATA) {
        //Send error mesage TODO
        return;
      } else if (written == NOT_DONE_SENDING){
        return;
      } else if (written == DONE_SENDING)
        curr->status = SEND_RESPONSE;
    }
    if (curr->status == SEND_RESPONSE) {
      switch (curr->to_do) {
        case PUT:
          return;
        default:
          return;
      }
    }
}

int main(int argc, char** argv) {

  if (argc != 2) {
    printf("Usage : ./server-reference <port>\n");
    exit(1);
  }

  set_up_signals();
  set_up_worker_list();
  set_up_gloabls(argv[1]);

	// Event loop
	while(!endSession) {
		struct epoll_event new_event;
    // printf("Waiting...\n");
    // printf("Waiting on fd %i\n", sock_fd);
		if(epoll_wait(epoll_fd, &new_event, 1, -1) > 0)
		{
      // printf("HAVE AN EVENT with fd = %i\n", new_event.data.fd);
			//Probably check for errors

			// New Connection Ready
			if(sock_fd == new_event.data.fd)
				accept_connections(&new_event, epoll_fd);
			else
				handle_data(&new_event);
		}
	}

  clean_up_globals();

  return 0;
}
