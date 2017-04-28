#pragma once

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/stat.h>
#include <errno.h>
#include <ctype.h>
#include <pthread.h>
#include <signal.h>

#define BUF_SIZE 1024

int interface_main();
void send_request(int sockFd, char *buffer, char *local);
size_t get_user_file_size(char *filename);
void write_binary_data(FILE *f, int sockFd, char *buffer);
void print_binary_data(FILE *f, int sockFd, char *buffer, size_t dataSize);
ssize_t write_all_from_socket_to_fd(int socket, int fd, ssize_t size);
bool check_ok(int sockFd, char *buffer);
ssize_t my_write(int socket, void *buffer, size_t count);
ssize_t my_read(int socket, void *buffer, size_t count);
