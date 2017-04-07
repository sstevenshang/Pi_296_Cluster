#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

int wait_for_input();
void print_usage();

int main(int argc, char const *argv[]) {
	printf("1234\n");
	wait_for_input(argv);
	printf("2345\n");
	return 0;
}

void print_usage(char const *argv[]) {
	printf("usage: ./%s [executable]\n", argv[0]);
}

int wait_for_input(char const *argv[]) {

	char* buffer = NULL;
	size_t size = 0;
	size_t i = 0;
	print_usage(argv);
	while((i = getline(&buffer, &size, stdin)) > 0) {
		buffer[i-1] = 0;
		puts(buffer);
		if (access(buffer, X_OK) == 0) {
			// execute(buffer);
			printf("executing %s\n", buffer);
		}
		print_usage(argv);
		free(buffer);
		buffer = NULL;
		size = 0;
	}
	return 0;
}
