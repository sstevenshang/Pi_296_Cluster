/**
 * Networking
 * CS 241 - Spring 2017
 */
#include "common.h"
#include "format.h"
#include "ctype.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static volatile int server_fd;

void do_delete(char* remote) {
  char request[4096];

  sprintf(request, "DELETE %s\n", remote);

  ssize_t count = write_all_to_socket(server_fd, request, strlen(request));

  if (count < (ssize_t) strlen(request)) {
    print_connection_closed();
    return;
  }

  shutdown_further_writes(server_fd);

  validate_server_response(server_fd);
}

void do_put(char* remote, char* local) {
  char request[4096];

  size_t size_of_file;

  if (SEND_TOO_LITTLE){
    size_of_file = get_file_size(local)+1;
  } else if (SEND_TOO_MUCH) {
    size_of_file = get_file_size(local)-1;
  } else
    size_of_file = get_file_size(local);

  if (!size_of_file) {
    print_error_message("open: : No such file or directory");
    return;
  }

  if (MALFORMED)
    sprintf(request, "PUT %s\n", remote);
  else
    sprintf(request, "PUT %s\n", remote);

  if (MALFORMED)  {
    shutdown_further_writes(server_fd);
    write_all_from_socket_to_fd(server_fd, STDOUT_FILENO, -1);
    close(server_fd);
    return;
  }

  ssize_t count = write_all_to_socket(server_fd, request, strlen(request));

  if (count < (ssize_t) strlen(request)) {
    print_connection_closed();
    return;
  }

  count = write_all_to_socket(server_fd, (char*)&size_of_file, sizeof(size_t));

  if (count < (ssize_t) sizeof(size_t)) {
    print_connection_closed();
    return;
  }

  FILE* fp = fopen(local, "r");
  int fd = fileno(fp);

  if (SEND_TOO_LITTLE) {
    write_all_from_socket_to_fd(fd, server_fd, size_of_file-1);
  } else if (SEND_TOO_MUCH) {
    write_all_from_socket_to_fd(fd, server_fd, size_of_file+1);
  } else
    write_all_from_socket_to_fd(fd, server_fd, size_of_file);

  shutdown_further_writes(server_fd);

  if (SEND_TOO_LITTLE | SEND_TOO_MUCH)
    write_all_from_socket_to_fd(server_fd, STDOUT_FILENO, -1);
  else
    validate_server_response(server_fd);

  close(fd);
}

void do_get(char* remote, char* local) {
  char request[4096];
  sprintf(request, "GET %s\n", remote);

  ssize_t count = write_all_to_socket(server_fd, request, strlen(request));

  if (count < (ssize_t) strlen(request)) {
    print_connection_closed();
    return;
  }

  shutdown_further_writes(server_fd);

  if (validate_server_response(server_fd))
    return;

  size_t message_length = get_message_length(server_fd);

  if (!message_length)
    return;

  FILE* file = fopen(local, "w");
  int fd = fileno(file);
  size_t written = write_all_from_socket_to_fd(server_fd, fd, message_length);

  if (written != message_length)
    remove(local);

  close(fd);
}

void do_list() {
  ssize_t count = write_all_to_socket(server_fd, "LIST\n", 5);

  if (count < 5) {
    print_connection_closed();
    return;
  }

  shutdown_further_writes(server_fd);

  if (validate_server_response(server_fd))
    return;

  size_t message_length = get_message_length(server_fd);

  if (!message_length)
    return;

  write_all_from_socket_to_fd(server_fd, STDOUT_FILENO, message_length);
}

char **parse_args(int argc, char **argv);
verb check_args(char **args, int argc);

int main(int argc, char **argv) {
  //Check the arguments. Get the verb/
  verb to_do = check_args(argv, argc);
  //Parse the arguments
  char** parsed = parse_args(argc, argv);

  if (!parsed) {
    exit(EXIT_FAILURE);
  }
  //Open up a connection with the server
  server_fd = connect_to_server(parsed[0], parsed[1]);

  if (server_fd == -1) {
    free(parsed);
    exit(EXIT_FAILURE);
  }

  switch(to_do) {
    case GET:
      do_get(parsed[3], parsed[4]);
      break;
    case PUT:
      do_put(parsed[3], parsed[4]);
      break;
    case DELETE:
      do_delete(parsed[3]);
      break;
    case LIST:
      do_list();
      break;
  }

  //Fully close the socket
  shutdown_further_reads(server_fd);
  close(server_fd);
  //Free parsed arguments
  free(parsed);
  //Exit
  exit(EXIT_SUCCESS);
}

/**
 * Given commandline argc and argv, parses argv.
 *
 * argc argc from main()
 * argv argv from main()
 *
 * Returns char* array in form of {host, port, method, remote, local, NULL}
 * where `method` is ALL CAPS
 */
char **parse_args(int argc, char **argv) {
  if (argc < 3) {
    return NULL;
  }

  char *host = strtok(argv[1], ":");
  char *port = strtok(NULL, ":");
  if (port == NULL) {
    return NULL;
  }

  char **args = calloc(1, (argc + 1) * sizeof(char *));
  args[0] = host;
  args[1] = port;
  args[2] = argv[2];
  // char *temp = args[2];
  // while (*temp) {
  //   *temp = toupper((unsigned char)*temp);
  //   temp++;
  // }
  if (argc > 3) {
    args[3] = argv[3];
  } else {
    args[3] = NULL;
  }
  if (argc > 4) {
    args[4] = argv[4];
  } else {
    args[4] = NULL;
  }

  return args;
}

/**
 * Validates args to program.  If `args` are not valid, help information for the
 * program is printed.
 *
 * args     arguments to parse
 *
 * Returns a verb which corresponds to the request method
 */
verb check_args(char **args, int argc) {
  if (args == NULL || argc < 3) {
    print_client_usage();
    exit(1);
  }

  char *command = args[2];

  if (strcmp(command, "LIST") == 0) {
    return LIST;
  }

  if (strcmp(command, "GET") == 0) {
    if (args[3] != NULL && args[4] != NULL) {
      return GET;
    }
    print_client_help();
    exit(1);
  }

  if (strcmp(command, "DELETE") == 0) {
    if (args[3] != NULL) {
      return DELETE;
    }
    print_client_help();
    exit(1);
  }

  if (strcmp(command, "PUT") == 0) {
    if (args[3] == NULL || args[4] == NULL) {
      print_client_help();
      exit(1);
    }
    return PUT;
  }

  // Not a valid Method
  print_client_help();
  exit(1);
}
