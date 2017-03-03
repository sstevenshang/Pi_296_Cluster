#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <arpa/inet.h>
#include "networkManager.h"

int main(int argc, char **argv)
{
  int socket_fd = setUpServer(NULL, "9001");
  sendBinaryFile(socket_fd, "./hi.out");
  return cleanUpServer(socket_fd);
}
