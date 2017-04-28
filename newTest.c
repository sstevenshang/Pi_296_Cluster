#include <stdio.h>
#include <unistd.h>

int main() {
	printf("Working (sleeping) for 5 seconds!\n");
	sleep(5);
	printf("Finished Working!\n");
	return 0;
}
