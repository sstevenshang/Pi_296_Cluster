#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include "node.h"
#include "master.h"
#include "worker.h"

int wait_for_input();
void print_usage();

int interface_main(int argc, char const *argv[]) {
	wait_for_input(argv);
	return 0;
}

void print_usage(char const *argv[]) {
	printf("usage: %s [executable]\n", argv[0]);
}

int wait_for_input(char const *argv[]) {
	char* buffer = NULL;
	size_t size = 0;
	size_t i = 0;
	master_main();
	printf("Enter executable name: ");
	while((i = getline(&buffer, &size, stdin)) > 0) {
		buffer[i - 1] = 0;
		printf("You entered: %s\n", buffer);
		if (access(buffer, X_OK) == 0) {
			// execute(buffer);
			printf("executing %s\n", buffer);
		} else {
			print_usage(argv);
		}
		printf("Enter executable name: ");
		free(buffer);
		buffer = NULL;
		size = 0;
	}
	return 0;
}
