#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include "networkManager.h"
#include <pthread.h>
int main(int argc, char **argv)
{
  int socket_fd = setUpClient("sp17-cs241-005.cs.illinois.edu", "9001");
  getBinaryFile(socket_fd, "./gotHi.out");
  return cleanUpClient(socket_fd);
}
