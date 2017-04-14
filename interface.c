#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include "node.h"
#include "master.h"
#include "worker.h"

int wait_for_input();
void print_usage();
int sendBinaryFile(int socket, char *name);

int interface_main(int argc, char const *argv[]) {
	printf("In client\n");
	wait_for_input(argv);
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

int sendBinaryFile(int socket, char *name) {
	int file = fileno(fopen(name, "rb"));
  	int bufSize = 2048;
  	char buf[bufSize]; // completely abritray, could be up to 65535 (2^16 -1)
  	int len = read(file, buf, bufSize); // holds if no data in pipeline
  	while (len != 0){  // len read something
    	if(len == -1) {
      		fprintf(stderr, "some error with reading from the fd %d, check for SIGPIPE\n", file);
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
    	fprintf(stderr, "some error with reading from the fd %d, check for SIGPIPE\n", file);
    	return 0;
  	}
  	close(file);
  	return 0;
}
