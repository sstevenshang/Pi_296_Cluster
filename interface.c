#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

int wait_for_input();
void print_usage();

int main(int argc, char const *argv[])
{
	wait_for_input();
	return 0;
}

int wait_for_input() {

	char* buffer = NULL;
	size_t size = 0;
	size_t i = 0;
	print_usage();
	while((i = getline(&buffer, &size, stdin)) > 0) {
		buffer[i-1] = 0;
		puts(buffer);
		if (access(buffer, X_OK) == 0) {
			// execute(buffer);
			printf("executing %s\n", buffer);
		}
		print_usage();
		free(buffer);
		buffer = NULL;
		size = 0;
	}
	return 0;
}

void print_usage() {
	printf("Enter executable name: ");
}