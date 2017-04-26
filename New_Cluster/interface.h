#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/stat.h>
#include <errno.h>

#define BUF_SIZE    1024

int interface_main(int argc, char **argv);
void send_request(int sockFd, char *buffer, char *local);
size_t get_file_size(char *filename);
void write_binary_data(FILE *f, int sockFd, char *buffer);
bool check_ok(int sockFd, char *buffer);
void print_binary_data(FILE *f, int sockFd, char *buffer, size_t dataSize);
ssize_t my_write(int socket, void *buffer, size_t count);
ssize_t my_read(int socket, void *buffer, size_t count);
