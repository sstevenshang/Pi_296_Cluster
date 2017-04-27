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
  // close(server_socket);

  // vector_destroy(worker_list);
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
  newWorker->file_buffer_pos = 0;
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
  newWorker->CPU_usage = -1;
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

    printf("Accepted connection from %s on file descriptor %i\n", connected_ip, new_fd);

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
    printf("Have a task coming in on file descriptor %i\n", e->data.fd);
    int vector_pos = find_worker_pos(e->data.fd);
    worker* curr;
    ssize_t check;

    if (vector_pos == -1)
      curr = interface;
    else
      curr = vector_get(worker_list, vector_pos);

    if (curr->status == START) {
      check = get_command(curr);
      switch (check) {
        case NOT_DONE_SENDING:
          return;
        case DONE_SENDING:
          curr->status = NEED_SIZE;
          printf("Succesfully got the header: %s\n", curr->command);
          if (curr->to_do == INTERFACE_PUT && interface_fd == -1) {
            //Add it to the interface list (for when > 1 interface). Remove from worker_list.
            //Later insted of checking if interface_fd is -1 we will check if its in workerlist
            interface_fd = curr->worker_fd;
      	    interface = curr;
      	    vector_erase(worker_list,find_worker_pos(e->data.fd));
          }
          break;
        default:
          fprintf(stderr, "There was a failure in parsing the header!\n");
          if (e->data.fd == interface_fd) {
            close(interface_fd);
            interface_fd = -1;
            fprintf(stderr, "This means the interface quit! Waiting on a new one...\n");
            return;
          }
      }
    }
    if (curr->status == NEED_SIZE) {
      check = get_size(curr);
      switch (check) {
        case NOT_DONE_SENDING:
          return;
        case DONE_SENDING:
          printf("Got the size of the file to be %zu\n", curr->file_size);
          curr->status = RECIEVING_DATA;
          break;
        default:
          fprintf(stderr, "There was a failure in getting the size!\n");
      }
    }
    if (curr->status == RECIEVING_DATA) {
      check = get_binary_data(curr);
      switch (check) {
        case NOT_DONE_SENDING:
          printf("Finished recieving binary data...\n");
          return;
        case DONE_SENDING:
          curr->status = FORWARD_DATA;
          break;
        default:
          fprintf(stderr, "There was a failure recieving data!\n");
      }
    }
    if (curr->status == FORWARD_DATA) {
      printf("Interfacing with the scheduler\n");
      //If the request is from the interface, forward the data to a worker
      if (e->data.fd == interface_fd) {
        task* new_task = make_task(curr);
        curr->fd_to_send_to = schedule(new_task, worker_list);
      }
      //If this is a response from a worker, forward the data to interface
      else {
        scheduler_remove_task(curr->worker_fd, curr->temp_file_name, worker_list);
        curr->fd_to_send_to  = interface_fd;
      }
      curr->status = FORWARDING_DATA;
    }
    if (curr->status == FORWARDING_DATA) {
      printf("Forwarding data...\n");
      ssize_t s = do_put(curr->fd_to_send_to, curr);
      reset_worker_for_parsing(curr);
      if (s != BIG_FAILURE)
        printf("Succesfully handled task\n");
      curr->status = START;
    }
}

int master_main() {
  setHandlers();
  setUpGlobals("9999");

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

//TODO dummy for now
// int schedule(task* t, vector* worker_list) {
//   (void) t;
//   worker* w = vector_get(worker_list, 0);
//   return w->worker_fd;
// }
//
// //TODO dummy for now
// void scheduler_remove_task(int worker_fd, char* filename, vector* worker_list) {
//   (void) worker_fd; (void) filename; (void) worker_list;
//   return;
// }

ssize_t do_put(int fd_to_send_to, worker* w) {
  int src_fd = w->temp_fd;
  size_t file_size = get_file_size(w->temp_file_name);
  char buff[4096];
  sprintf(buff, "PUT %s\n", w->temp_file_name);
  //Send the header
  if (write_all_to_socket(fd_to_send_to, buff, strlen(buff)) != (ssize_t) strlen(buff)) {
    perror(NULL);
    fprintf(stderr, "There was an issue sending the header to fd %i\n", fd_to_send_to);
    return BIG_FAILURE;
  }
  //Send the size
  if (write_all_to_socket(fd_to_send_to, (char*) &file_size, sizeof(size_t)) != (ssize_t) sizeof(size_t)) {
    perror(NULL);
    fprintf(stderr, "There was an issue sending the file size to fd %i\n", fd_to_send_to);
    return BIG_FAILURE;
  }

  lseek(src_fd, 0, SEEK_SET);

  ssize_t ret;
  while (file_size) {
    ret = (file_size < 4096) ? read(src_fd, buff, file_size) : read(src_fd, buff, 4096);
    if (ret == -1) {
      if (errno == EINTR)
        continue;
      else if (errno == EWOULDBLOCK || errno == EAGAIN) {
        fprintf(stderr, "There was an error (blocking) reading the file: %s\n", w->temp_file_name);
        return BIG_FAILURE;
      }
      perror(NULL);
      fprintf(stderr, "There was an unknown error reading the file: %s\n", w->temp_file_name);
      return BIG_FAILURE;
    } else if (ret == 0) {
      fprintf(stderr, "There was an error (read 0) reading the file: %s\n", w->temp_file_name);
      return BIG_FAILURE;
    }

    ssize_t check_write = write_all_to_socket(fd_to_send_to, buff, ret);
    if (check_write == -1 || !check_write || check_write != ret) {
      fprintf(stderr, "There was an error writing the file over the network: %s to fd %i\n", w->temp_file_name, fd_to_send_to);
      return BIG_FAILURE;
    }
    file_size -= ret;
  }

  return DONE_SENDING;
}

//Frees a task
void free_task(task* t) {
  free(t->file_name);
  free(t);
}

//Makes a task with the given parsing information in the worker
task* make_task(worker* w) {
  task* t = malloc(sizeof(task));
  t->file_name = strdup(w->temp_file_name);
  return t;
}

int open_with_all_permission(char* filename) {
  return open(filename, O_CREAT | O_RDWR | O_TRUNC, S_IRWXU | S_IRWXG | S_IRWXO);
}

//Retrieves the given data and puts it into the file_name
ssize_t get_binary_data(worker* curr) {
  //If we haven't opened a file descriptor of this file, open one
  if (curr->temp_fd == -1) {
    curr->temp_fd = open_with_all_permission(curr->temp_file_name);
  }

  int fd = curr->worker_fd;
  ssize_t ret;
  char buff[4096];

  while (curr->file_size > 0) {
    ret = (curr->file_size < 4096) ? read(fd, buff, curr->file_size) : read(fd, buff, 4096);
    if (ret == -1) {
      if (errno == EWOULDBLOCK || errno == EAGAIN)
        return NOT_DONE_SENDING;
      else if (errno == EINTR)
        continue;
      perror(NULL);
      fprintf(stderr, "There was an unknown issue in recieving data!\n");
      return BIG_FAILURE;
    } else if (ret == 0) {
      fprintf(stderr, "There was an issue (not enough bytes) in recieving data!\n");
      return BIG_FAILURE;
    }

    ssize_t check_write = write_all_to_socket(curr->temp_fd, buff, ret);
    if (check_write == -1 || !check_write) {
      fprintf(stderr, "There was an issue in writing recieved data to disk!\n");
      return BIG_FAILURE;
    }
    curr->file_size -= ret;
  }

  return DONE_SENDING;
}

//Retrieves the size, setting the file_size
ssize_t get_size(worker* w) {
  int fd = w->worker_fd;
  char* filesize = (char*) &w->file_size;
  ssize_t ret;
  char buff;

  while (1) {
      ret = read(fd, &buff, 1);
      if (ret == -1) {
        if (errno == EWOULDBLOCK || errno == EAGAIN)
          return NOT_DONE_SENDING;
        else if (errno == EINTR)
          continue;
        perror(NULL);
        fprintf(stderr, "%s\n", "There was an unknown error reading for the size!\n");
        return BIG_FAILURE;
      } else if (ret == 0) {
        fprintf(stderr, "%s\n", "There was an error (not enough bytes) reading for the size!\n");
        return BIG_FAILURE;
      }

      filesize[w->size_buffer_pos++] = buff;

      //Watch for buffer overflow
      if (w->size_buffer_pos == sizeof(size_t)) {
        return DONE_SENDING;
      }
  }
}

//Gets the header up until '\n'. Sets the char* temp_file_name and command to_do.
ssize_t get_command(worker* w) {
  int fd = w->worker_fd;
  ssize_t ret;
  char buff;

  while (1) {
      ret = read(fd, &buff, 1);
      if (ret == -1) {
        if (errno == EWOULDBLOCK || errno == EAGAIN)
          return NOT_DONE_SENDING;
        else if (errno == EINTR)
          continue;
        perror(NULL);
        fprintf(stderr, "%s\n", "There was an unknown error reading the header\n");
        return BIG_FAILURE;
      } else if (ret == 0) {
        fprintf(stderr, "%s\n", "There was an error (not enough) reading the header!\n");
        return BIG_FAILURE;
      }
      if (buff == '\n')
        break;
      w->command[w->command_size++] = buff;

      //Watch for buffer overflow
      if (w->command_size > 1025) {
        fprintf(stderr, "%s\n", "There was an error (too many characters before reading newline) reading the header!\n");
        return BIG_FAILURE;
      }
  }

  if (strncmp("INTERFACE_PUT", w->command, strlen("INTERFACE_PUT")) == 0) {
    w->to_do = INTERFACE_PUT;
  } else if (strncmp("PUT", w->command, 3) == 0) {
    w->to_do = PUT;
  } else {
    fprintf(stderr, "Invalid Header: %s\n", w->command);
    return BIG_FAILURE;
  }

  unsigned idx = 0;
  for (; idx < strlen(w->command); idx++) {
    if (w->command[idx] == ' ') {
      idx++;
      break;
    }
  }

  w->temp_file_name = strdup(w->command+idx);

  if (strlen(w->temp_file_name) == 0){
    fprintf(stderr, "Invalid Header: %s\n", w->command);
    return BIG_FAILURE;
  }

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

int schedule(task* t, vector* worker_list) {

    size_t num_worker = vector_size(worker_list);
    double min_load_factor = 100000000;
    worker* best_worker = NULL;

    for (size_t i=0; i < num_worker; i++) {
        worker* this_worker = vector_get(worker_list, i);
        // MUTEX LOCK WHENCE INTEGRATED HEARTBEAT
        if (this_worker->alive) {
            double load_factor = this_worker->CPU_usage * 10 + vector_size(this_worker->tasks);
            //Make sure laod factor is positive because nodes upon initialization are set to -1.
            //First reception of the heartbeat allows a node to have work allocated to it
            if (load_factor < min_load_factor && load_factor > 0) {
                min_load_factor = load_factor;
                best_worker = this_worker;
            }
        }
        // MUTEX UNLOCK WHENCE INTEGRATED HEARTBEAT
    }
    //Dummy for now until we have heartbeats sending usage stats. Remove when heartbeat implemented.
    best_worker = vector_get(worker_list, 0);

    if (best_worker == NULL) {
        return -1;
    }
    vector_push_back(best_worker->tasks, t);
    return best_worker->worker_fd;
}

void scheduler_remove_task(int worker_fd, char* filename, vector* worker_list) {
    size_t num_worker = vector_size(worker_list);
    worker* target_worker = NULL;
    for (size_t i=0; i < num_worker; i++) {
        worker* this_worker = vector_get(worker_list, i);
        if (this_worker->worker_fd == worker_fd) {
            target_worker = this_worker;
            break;
        }
    }
    if (target_worker == NULL) {
        printf("Worker %d not found when removing %s\n", worker_fd, filename);
        return;
    }
    vector* worker_tasks = target_worker->tasks;
    size_t num_task = vector_size(worker_tasks);
    task* target_task = NULL;
    for (size_t i=0; i < num_task; i++) {
        task* this_task = vector_get(worker_tasks, i);
        if (strncmp(this_task->file_name, filename, strlen(this_task->file_name)) == 0) {
            target_task = this_task;
            vector_erase(worker_tasks, i);
            break;
        }
    }
    if (target_task == NULL) {
        printf("Task %s not found on worker %d\n", filename, worker_fd);
    }
}
