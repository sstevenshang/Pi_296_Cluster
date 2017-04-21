#ifndef _INTERFACE_H_
#define _INTERFACE_H_

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include "node.h"
#include "master.h"
#include "worker.h"

int wait_for_input();
void print_usage();
int setUpClient(char *addr, char *port);
int cleanUpClient(int socket);
int sendBinaryFile(int socket, char *name);
ssize_t my_write(int socket, void *buffer, size_t count);
void write_binary_data(FILE *f, int sockFd, char *buffer);
int interface_main(int argc, char const *argv[]);

#endif
