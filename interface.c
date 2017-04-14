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

int interface_main(int argc, char const *argv[]) {
    printf("In client\n");

	char* defaultMaster = "sp17-cs241-005.cs.illinois.edu";
    char tempBuf[1024];
    fprintf(stdout, "Type address to connect (press enter to connect to default master: %s)\n", defaultMaster);
    size_t bytesRead = read(fileno(stdin), tempBuf, 1023);
    if (bytesRead == 0) {
		fprintf(stdout, "Using default address %s\n", defaultMaster); strcpy(tempBuf, defaultMaster);
	} else {
    	tempBuf[bytesRead-1] = '\0';
    	printf("Using address %s\n", tempBuf);
    }
    socketFd = setUpClient(tempBuf, defaultInterfacePort);
    if (socketFd == -1) {
      fprintf(stderr, "Failed connection, try again later\n");
      return 0;
    }

	wait_for_input(argv);

    cleanUpClient(socketFd);
    return 0;
}

void print_usage(char const *argv[]) {
    printf("usage: %s [executable]\n", argv[0]);
}

int wait_for_input(char const *argv[]) {
    char buffer[16];
    size_t numExec = 0;
    printf("Enter number of executable(s): ");
    scanf("%zu", &numExec);
    printf("%zu executable(s).\nEnter executable name: ", numExec);
    for (size_t i = 0; i < numExec; i++) {
	scanf("%s", buffer);
	printf("You entered: %s\n", buffer);
	if (access(buffer, X_OK) == 0) {
	    // execute(buffer);
	    printf("executing %s\n", buffer);
	} else {
	    print_usage(argv);
	}
	printf("Enter executable name: ");
    }
    return 0;
}

int setUpClient(char *addr, char *port) {
  	int s;
  	int socketFd = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0);
  	struct addrinfo hints, *result;
  	memset(&hints, 0, sizeof(struct addrinfo));
  	hints.ai_family = AF_INET;
  	hints.ai_socktype = SOCK_STREAM;
  	s = getaddrinfo(addr, port, &hints, &result);
  	fprintf(stdout, "using %s and %s\n", addr, port);
  	if (s != 0){
    	fprintf(stderr, "failed to find %s at port %s\n", addr, port);
    	return -1;
  	}
  	if (connect(socketFd, result->ai_addr, result->ai_addrlen)==-1){
    	if (errno != EINPROGRESS){
      		perror("connect");
      		return -1;
    	}
  	}
  	fprintf(stdout, "Found connection on fd %d\n", socket_fd);
  	return socketFd;
}

int cleanUpClient(int socket) {
  	shutdown(socket, SHUT_WR);
  	close(socket);
  	return 0;
}

int sendBinaryFile(int socket, char *name) {
    int file = fileno(fopen(name, "rb"));
    int bufSize = 2048;
    char buf[bufSize]; // completely abritray, could be up to 65535 (2^16 -1)
    int len = read(file, buf, bufSize); // holds if no data in pipeline
    while (len != 0) {  // len read something
    	if (len == -1) {
      	    fprintf(stderr,
      	      "some error with reading from the fd %d, check for SIGPIPE\n",
      	      file);
      	    return 0;
    	}
    	int s = write(socket, buf, len);
    	if (s == -1) {
	    close(file);
	    return 0;
	}
    	while (s != len) { // as long as write hasn't written all bytes
      	    int k = write(socket, buf + s, len - s);
      	    if (k == -1) {
		close(file);
		return 0;
	    }
      	    s += k;
    	}
    	s = read(socket, NULL, 0); // ingnoring, putting more data
    	len = read(file, buf, bufSize);
    }
    if (len == -1) {
    	fprintf(stderr,
    	  "some error with reading from the fd %d, check for SIGPIPE\n",
    	  file);
    	return 0;
    }
    close(file);
    return 0;
}
