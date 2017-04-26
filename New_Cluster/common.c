#include "common.h"

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
    // printf("curr_read is %zi\n", curr_read);
    if (curr_read == -1) {
      // printf("%s\n", strerror(errno));
      if (errno == EINTR)
        continue;
      else if (errno == EWOULDBLOCK || errno == EAGAIN || errno == 11){
        // printf("They aren't done sending. %zi bytes left\n", t->file_size);
        return NOT_DONE_SENDING;
      }
      return -1;
    } else if (curr_read == 0) {
        break;
    }

    // ssize_t written = write_all_to_socket(fd, buffer, curr_read);
    ssize_t written = write(fd, buffer, curr_read);
    // printf("Wrote %zi bytes!\n", written);
    t->file_size -= curr_read;
    total_read += curr_read;

    if (written == - 1 && errno == EPIPE)
      return -1;
  }
  // printf("File size is %zu\n",t->file_size);
  if (t->file_size != 0 || read(socket, buffer, 1) > 0) {
    // printf("BAD DATA SIZE\n");
    return TOO_LITTLE_DATA;
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

int get_verb(task* to_do) {
  
      if (strncmp(to_do->header, "INTERFACE_PUT", 13) == 0) {
        return 1;
      } else if (strncmp(to_do->header, "MASTER_TO_WORKER_PUT", 20) == 0) {
        return 2;
      } else if (strncmp(to_do->header, "WORKER_TO_MASTER_PUT", 20) == 0) {
        return 3;
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

// This is the destructor function for string element.
// Use this as destructor callback in vector.
void string_destructor(void *elem) { free(elem); }

// This is the default constructor function for string element.
// Use this as a default constructor callback in vector.
void *string_default_constructor(void) {
  // A single null byte
  return calloc(1, sizeof(char));
}

vector* string_vector_create() {
  return vector_create(string_copy_constructor, string_destructor, string_default_constructor);
}

void* task_copy_constructor(void* elem) {
  task* to_copy = elem;
  task* new_task = malloc(sizeof(task));
  new_task->request = to_copy->request;
  new_task->to_do = to_copy->to_do;
  new_task->status = to_copy->status;
  new_task->file_size = to_copy->file_size;
  new_task->size_buffer_pos = to_copy->size_buffer_pos;
  new_task->head_size = to_copy->head_size;
  return new_task;
}

void task_destructor(void* elem) {
  free(elem);
}

int set_up_server(char* port) {
  int s;
  // Create the socket as a nonblocking socket
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

  // struct sockaddr_in sin;
  // socklen_t socklen = sizeof(sin);
  // if (getsockname(sock_fd, (struct sockaddr *)&sin, &socklen) == -1)
  //   perror("getsockname");
  // else
  //   printf("Listening on port number %d\n", ntohs(sin.sin_port));

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

    //Free the data allocated by getaddrinfo
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

ssize_t write_all_from_socket_to_fd(int socket, int fd, size_t size) {
  char buffer[4096];
  ssize_t curr_read;
  ssize_t total_read = 0;
  errno = 0;
  ssize_t initial = size;
  while (size) {
    curr_read = (4096 < size || initial == -1) ? read_all_from_socket(socket, buffer, 4096) : read_all_from_socket(socket, buffer, size);
    // printf("curr_read is %zi\n", curr_read);
    if (curr_read == -1) {
      // printf("%s\n", strerror(errno));
      if (errno == EINTR || errno == EWOULDBLOCK || errno == EAGAIN)
        continue;
      // else if (errno == EWOULDBLOCK || errno == EAGAIN)
      //   return NOT_DONE_SENDING;
      return -1;
    } else if (curr_read == 0) {
        break;
    }
    size -= curr_read;
    total_read += curr_read;

    ssize_t written = write_all_to_socket(fd, buffer, curr_read);

    if (written == - 1 && errno == EPIPE)
      return -1;
  }

  if (total_read < initial && initial != -1) {
    print_too_little_data();
  }

  if (initial != -1) {
    int flags = fcntl(socket, F_GETFL, 0);
    fcntl(socket, F_SETFL, flags | O_NONBLOCK);
  }

  if (read(socket, buffer, 1) && initial != -1) {
    print_recieved_too_much_data();
  }
  // printf("Wrote %zi\n", total_read);
  if (initial == -1) {
    return DONE_SENDING;
  }

  return total_read;
}

//Gets the error message is error
void new_print_error_message(int socket) {
  int flags = fcntl(socket, F_GETFL, 0);
  fcntl(socket, F_SETFL, flags | O_NONBLOCK);
  char buf;

  while (read(socket, &buf, 1)) {
    printf("%c", buf);
  }
}

//0 on success, 1 on fail
int validate_server_response(int socket) {
  // char buffer[6];
  // ssize_t count = read_all_from_socket(socket, buffer, 3);
  // if (count < 3) {
  //   print_invalid_response();
  //   return 1;
  // }
  // if (strncmp("OK\n", buffer, 3) != 0) {
  //   //Bring us to the start of the error message
  //   count = read_all_from_socket(socket, buffer+3, 3);
  //   //Make sure the response says error
  //   if (count < 3 || strncmp("ERROR\n", buffer, 6) != 0) {
  //     print_invalid_response();
  //     return 1;
  //   }
  //   new_print_error_message(socket);
  //   return 1;
  // }
  write_all_from_socket_to_fd(socket, STDOUT_FILENO, -1);
  return 0;
}

size_t get_message_length(int socket) {
  size_t message_length;
  ssize_t count = read_all_from_socket(socket, (char*) &message_length, sizeof(size_t));
  if (count < (ssize_t) sizeof(size_t)) {
    print_invalid_response();
    return 0;
  }
  return message_length;
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
