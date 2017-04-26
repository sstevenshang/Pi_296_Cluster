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

//return 1 on fail, 0 on success
int get_verb(int sfd, task* to_do) {
  char b;
  int idx = 0;
  ssize_t read;

  for (;idx < HEADER_BUFFER_SIZE; idx++)
    if (!idx)
      break;

  while ( (read = read_all_from_socket(sfd, &b, 1)) != -1) {
    if (idx > 1024)
      return INVALID_COMMAND;

    to_do->header[idx++] = b;
    to_do->head_size++;
    if (to_do->head_size > 1024) {
      return INVALID_COMMAND;
    }
    // printf("%s\n%i\n",to_do->header, idx);
    if (idx > 3) {
      if (strncmp(to_do->header, "PUT ", 4) == 0) {
        to_do->to_do = PUT;
        return 0;
      } else if (strncmp(to_do->header, "GET ", 4) == 0) {
        to_do->to_do = GET;
        return 0;
      }
      if (idx > 4) {
        if (strncmp(to_do->header, "LIST\n", 5) == 0) {
          to_do->to_do = LIST;
          return 0;
        }
        if (idx > 6) {
          if (strncmp(to_do->header, "DELETE ", 7) == 0) {
            to_do->to_do = DELETE;
            return 0;
          } else {
            return INVALID_COMMAND;
          }
        }
      }
    }
  }
  if (! read) {
    return INVALID_COMMAND;
  }
  return 1;
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
