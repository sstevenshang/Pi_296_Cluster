#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <arpa/inet.h>
#include "networkManager.h"

int main(int argc, char **argv){
    int running = setUpServer(NULL, "9001");
    while(running == -1){
	running = setUpServer(NULL, "9001");
    }
    fprintf(stdout, "server running\n");
    while(running){
      int testVal1 = addAnyIncomingConnections();
      if(testVal1 != -1){ fprintf(stdout, "added a connection\n");}
      //slaveManager();

    }
//  int running = 1;
//  char buf[100];
//  while(running){
//    fprintf(stdout, "type 1 to search for incoming connections, 2 to send a data file, 3 to send a exec file\n");
//    size_t bytesRead = read(0, buf, 99);
 //   buf[bytesRead] = '\0';
 //   if(bytesRead != 0){ //some input from stdin
 //     
 //   }
    int socket_fd = setUpServer(NULL, "9001");
    sendBinaryFile(socket_fd, "./hi.out");
 // }
  return cleanUpServer(socket_fd);
}
