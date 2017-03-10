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
  char myChk = (char)(0x09);
  char myEOF = (char)(0x05);
  char* toSend = RECEIVEME;
  char headedMessage[strlen(toSend) + 1 + 1];
  headedMessage[0] = myChk;
  strcat(headedMessage+1, toSend);
  fprintf(stdout, "sending: %s\n", headedMessage);
  write(socket_fd, headedMessage, strlen(headedMessage));
  sleep(1);
  headedMessage[0] = myEOF;
  headedMessage[1] = '\0';
  fprintf(stdout, "now sending EOF: %s\n", headedMessage);
  write(socket_fd, headedMessage, strlen(headedMessage));
  while(1){};
//  getBinaryFile(socket_fd, "./gotHi.out");
  return cleanUpClient(socket_fd);
  //return 1;
}
