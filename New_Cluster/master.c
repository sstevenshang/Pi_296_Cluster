#include "master.h"

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

  vector_destroy(worker_list);
}

void setUpGlobals(char* port) {
  server_socket = set_up_server(port);
	epoll_fd = epoll_create(1);
  worker_list = vector_create(NULL, NULL, NULL);

	if(epoll_fd == -1) {
    cleanGlobals();
    perror("epoll_create()");
    exit(1);
  }

	struct epoll_event event;
	event.data.fd = server_socket;
	event.events = EPOLLIN | EPOLLET;
	if(epoll_ctl(epoll_fd, EPOLL_CTL_ADD, server_socket, &event)) {
    cleanGlobals();
    perror("epoll_ctl()");
    exit(1);
	}

  char dummy[] = "./master_tempXXXXXX";
  temp_directory = strdup(mkdtemp(dummy));
  chdir(temp_directory);
  printf("Storing files in %s\n", temp_directory);
}

void reset_worker_for_parsing(worker* newWorker) {
  newWorker->temp_fd = -1;
  for (int i = 0; i < COMMAND_BUF_SIZE; i++)
    newWorker->command[i] = 0;
  newWorker->command_size = 0;
  newWorker->size_buffer_pos = 0;
  newWorker->status = START;
  if (newWorker->temp_fd != -1)
    close(newWorker->temp_fd);
  newWorker->temp_fd = -1;
  newWorker->file_size = 0;
  if (newWorker->temp_file_name)
    free(newWorker->temp_file_name);
  newWorker->temp_file_name = NULL;
}

worker* create_worker(int fd, char* IP){
  worker* newWorker = (worker*)malloc(sizeof(worker));
  newWorker->alive = 1;
  newWorker->tasks = vector_create(NULL, NULL, NULL);
  newWorker->worker_fd = fd;
  newWorker->IP = strdup(IP);
  //Note these two are essential to initialization
  newWorker->temp_file_name = NULL;
  newWorker->temp_fd = -1;
  reset_worker_for_parsing(newWorker);
  return newWorker;
}

//Make sure you shutdown + close worker_fd before calling.
void free_worker(worker* to_free) {
  vector_destroy(to_free->tasks);
  free(to_free->IP);
  if (to_free->temp_file_name)
    free(to_free->temp_file_name);
  free(to_free);
  to_free = NULL;
}

ssize_t find_worker_pos(int fd){
  size_t i = 0;
  while(i < vector_size(worker_list)){
    if(((worker*)vector_get(worker_list, i))->worker_fd == fd){
      return i;
    }
    i++;
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

    if (vector_pos == -1)
      curr = interface;
    else
      curr = vector_get(worker_list, vector_pos);

    if(interface_fd == -1){
	    interface_fd = curr->worker_fd;
	    interface = curr;
	    vector_erase(worker_list,find_worker_pos(e->data.fd));
    }

    if (curr->status == START) {
      check = get_command(curr);
      switch (check) {
        case NOT_DONE_SENDING:
          return;
        case DONE_SENDING:
          curr->status = NEED_SIZE;
          break;
        default:
          fprintf(stderr, "There was a catastrophic failure!\n");
      }
    }
    if (curr->status == NEED_SIZE) {
      check = get_size(curr);
      switch (check) {
        case NOT_DONE_SENDING:
          return;
        case DONE_SENDING:
          curr->status = RECIEVING_DATA;
          break;
        default:
          fprintf(stderr, "There was a catastrophic failure!\n");
      }
    }
    if (curr->status == RECIEVING_DATA) {
      check = get_binary_data(curr);
      switch (check) {
        case NOT_DONE_SENDING:
          return;
        case DONE_SENDING:
          curr->status = FORWARD_DATA;
          break;
        default:
          fprintf(stderr, "There was a catastrophic failure!\n");
      }
    }
    if (curr->status == FORWARD_DATA) {
      int fd_to_send_to;
      //If the request is from the interface, forward the data to a worker
      if (e->data.fd == interface_fd) {
        task* new_task = make_task(curr);
        fd_to_send_to = schedule(new_task, worker_list);
      }
      //If this is a response from a worker, forward the data to interface
      else {
        scheduler_remove_task(curr->worker_fd, curr->temp_file_name, worker_list);
        fd_to_send_to = interface_fd;
      }
      do_put(fd_to_send_to, curr);
      reset_worker_for_parsing(curr);
      curr->status = START;
    }
}

int master_main(int argc, char** argv) {
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
  MASTER_UTILS
*/

ssize_t transfer_fds(int socket, int fd, worker* t) {
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

    ssize_t written = write_all_to_socket(fd, buffer, curr_read);
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

//TODO dummy for now
int schedule(task* t, vector* worker_list) {
  (void) t;
  worker* w = vector_get(worker_list, 0);
  return w->worker_fd;
}

//TODO dummy for now
void scheduler_remove_task(int worker_fd, char* filename, vector* worker_list) {
  (void) worker_fd; (void) filename; (void) worker_list;
  return;
}

void do_put(int fd, worker* w) {
  (void) fd;
  (void) w;
}

//Frees a task
void free_task(task* t) {
  (void) t;
}

//Makes a task with the given parsing information in the worker
task* make_task(worker* w) {
  (void) w;
  return NULL;
}

//Retrieves the given data and puts it into the file_name
ssize_t get_binary_data(worker* curr) {
  (void) curr;
  return DONE_SENDING;
}
//Retrieves the size, setting the file_size
ssize_t get_size(worker* curr) {
  (void) curr;
  return DONE_SENDING;
}

//Gets the header up until '\n'. Sets the filename and command.
ssize_t get_command(worker* w) {
  (void) w;
  return DONE_SENDING;
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
        if (errno == EINTR || errno == EWOULDBLOCK || errno == EAGAIN)
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
