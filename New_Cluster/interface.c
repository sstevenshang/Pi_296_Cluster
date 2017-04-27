#include "interface.h"

static int running = 1;
static int sockFd;

void* output_reciever(void* elem) {
  pthread_detach(pthread_self());
  char buff[BUF_SIZE];
  int idx = 0;
  char b;
  size_t file_size = 0;

  while(running) {
    do {
      //Add parsing of filename in the future to track what requests sent out come back.
      //^^^^^^ Central to master fault tolerance. When a filename is recieved back,
      //remove it from the running vector of requests
      read(sockFd, &b, 1);
      if (b != '\n')
        buff[idx++] = b;
      else
        break;
    } while (1);

    //Get the size
    for (unsigned i = 0; i < sizeof(size_t); i++) {
      read(sockFd, &b, 1);
      ((char*)&file_size)[i] = b;
    }
    // write_all_from_socket_to_fd(sockFd, 1, -1);
    write_all_from_socket_to_fd(sockFd, 1, file_size);
    idx = 0;
  }
  pthread_exit(NULL);
}

int interface_main() {
    sockFd = socket(AF_INET, SOCK_STREAM, 0);
    struct addrinfo hints, *result;
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    int ga = getaddrinfo("127.0.0.1", "9999", &hints, &result);
    if (ga != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(ga));
        freeaddrinfo(result);
        exit(1);
    }
    if(connect(sockFd, result->ai_addr, result->ai_addrlen) == -1){
        perror("connect");
        freeaddrinfo(result);
        exit(2);
    }
    char *buffer = malloc(BUF_SIZE);
    int max = 1024;
    char *name = calloc(1, max);
    pthread_t child_thread;
    pthread_create(&child_thread, NULL, output_reciever, NULL);

    while (running) {
      printf("Enter an executable file name:\n");

      while (1) { /* skip leading whitespace */
        int c = getchar();
        if (c == EOF) break; /* end of file */
        if (!isspace(c)) {
             ungetc(c, stdin);
             break;
        }
      }

      int i = 0;
      while (1) {
        int c = getchar();
        if (isspace(c) || c == EOF) { /* at end, add terminating zero */
            name[i] = 0;
            break;
        }
        name[i] = c;
        i++;
        if (i > max) {
          fprintf(stderr, "%s\n", "Filename is larger than allowed (consider a shorter name)");
          exit(1);
        }
      }

      //Keep a running string vector of filenames sent out
      //Send the request
      strcpy(buffer, "INTERFACE_PUT");
      if (access(name, R_OK) != 0) {
          fprintf(stderr, "%s\n", "INVALID FILE");
          free(name);
          free(buffer);
          freeaddrinfo(result);
          exit(1);
      }
      send_request(sockFd, buffer, name);
      FILE *f = fopen(name, "r");
      size_t s = get_user_file_size(name);
      my_write(sockFd, (void *)&s, sizeof(size_t));
      write_binary_data(f, sockFd, buffer);
    }

    free(buffer);
    free(name);
    freeaddrinfo(result);
    close(sockFd);
    return 0;
}

bool check_ok(int sockFd, char *buffer) {
    buffer[0] = '\0';
    size_t s = my_read(sockFd, buffer, 1);
    buffer[s] = '\0';
    if (buffer[0] == 'O') { // OK
        s = my_read(sockFd, buffer, 2);
    } else { // ERROR
        s = my_read(sockFd, buffer, 5);
        buffer[0] = '\0';
        s = my_read(sockFd, buffer, BUF_SIZE);
        buffer[s] = '\0';
        printf("%s", buffer);
        free(buffer);
        return false;
    }
    return true;
}


ssize_t write_all_from_socket_to_fd(int socket, int fd, ssize_t size) {
  char buffer[4096];
  ssize_t curr_read;
  ssize_t total_read = 0;

  while (size) {
    curr_read = (4096 < size || size < 0) ? read(socket, buffer, 4096) : read(socket, buffer, size);
    // printf("curr_read is %zi\n", curr_read);
    if (curr_read == -1) {
      // printf("%s\n", strerror(errno));
      if (errno == EINTR || errno == EWOULDBLOCK || errno == EAGAIN)
        continue;
      return -1;
    } else if (curr_read == 0) {
        break;
    }
    size -= curr_read;
    total_read += curr_read;

    write(fd, buffer, curr_read);
  }

  return total_read;
}

void send_request(int sockFd, char *buffer, char *local) {
    strcat(buffer, " ");
    strcat(buffer, local);
    strcat(buffer, "\n");
    my_write(sockFd, buffer, strlen(buffer));
}

size_t get_user_file_size(char *filename) {
    struct stat buf;
    stat(filename, &buf);
    return buf.st_size;
}

void write_binary_data(FILE *f, int sockFd, char *buffer) {
    size_t s = 1;
    while (s != 0) {
        buffer[0] = '\0';
        s = fread(buffer, 1, BUF_SIZE, f);
        if (s != 0) {
            my_write(sockFd, buffer, s);
        }
    }
}

void print_binary_data(FILE *f, int sockFd, char *buffer, size_t dataSize) {
    size_t s = 1;
    size_t size = 0;
    while (s != 0) {
        buffer[0] = '\0';
        s = my_read(sockFd, buffer, BUF_SIZE);
        size += s;
        if (s != 0) {
            write(1, buffer, s);
        }
    }
}

ssize_t my_write(int socket, void *buffer, size_t count) {
    size_t c = count;
    int y = 0;
    do {
        y = write(socket, buffer, count);
        if (y != -1) {
            buffer += y;
            count -= y;
        }
    } while ((count > 0 && 0 < y) || (y == -1 && errno == EINTR));
    if (y == -1 && errno != EINTR) {
        return -1;
    }
    return c - count;
}

ssize_t my_read(int socket, void *buffer, size_t count) {
    size_t c = count;
    int y = 0;
    do {
        y = read(socket, buffer, count);
        if (y != -1) {
            buffer += y;
            count -= y;
        }
    } while ((count > 0 && 0 < y) || (y == -1 && errno == EINTR));
    if (y == -1 && errno != EINTR) {
        return -1;
    }
    return c - count;
}
