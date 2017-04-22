#include "interface.h"
#define BUF_SIZE 1024
/*
int wait_for_input();
void print_usage();
int setUpClient(char *addr, char *port);
int cleanUpClient(int socket);
int sendBinaryFile(int socket, char *name);
ssize_t my_write(int socket, void *buffer, size_t count);
void write_binary_data(FILE *f, int sockFd, char *buffer);
*/
static char* defaultMaster = "sp17-cs241-005.cs.illinois.edu";
static char master_addr[1024];
char buffer[4096];
size_t bufferSize;
int pos;
int socketFd;
int tmpFP;

int interface_main(int argc, char const *argv[]) {
    printf("In client\n");

    char tempBuf[1024];
    fprintf(stdout, "Type address to connect (press enter to connect to default master: %s)\n", defaultMaster);
    size_t bytesRead = read(fileno(stdin), tempBuf, 1023);
    if (tempBuf[0] == '\n') {
		fprintf(stdout, "Using default address %s\n", defaultMaster);
        strcpy(tempBuf, defaultMaster);
	} else {
    	tempBuf[bytesRead - 1] = '\0';
    	printf("Using address %s\n", tempBuf);
    }
    strcpy(master_addr, tempBuf);
    socketFd = setUpClient(tempBuf, defaultInterfacePort);
    if (socketFd == -1) {
        fprintf(stderr, "Failed connection, try again later\n");
        return 0;
    }

    wait_for_input(argv, socketFd);

    cleanUpClient(socketFd);
    return 0;
}

int handleData() {
    char tempBuf[1024];
    switch (pos) {
    case 0: {
        puts("case0");
	ssize_t bytesRead = read(socketFd, buffer, bufferSize);
	size_t nameLen = strlen(buffer);
	if (nameLen <= 1) { return -1; }
        strcpy(tempBuf, buffer);
        fprintf(stdout, "gotten filename is %s\n", tempBuf);
	//tmpFP = fopen(tempBuf, "wb+");
	tmpFP = open(tempBuf, O_CREAT | O_RDWR, S_IWGRP | S_IWUSR);
	if (tmpFP == -1) { return -1; }
	pos = 1;
        write(tmpFP, buffer + nameLen + 1, bytesRead - 1 - nameLen);
    } case 1: {
puts("case1");
        ssize_t bytesRead = readSocketIntoBuf(socketFd, buffer, bufferSize);
        if(bytesRead != 0){
	    puts("reading more data");
	    puts(buffer);
	    write(tmpFP, buffer, bytesRead);
	    return 1;
	}
    }
    }
    return 0;
}

void print_usage(char const *argv[]) {
    printf("usage: %s [executable]\n", argv[0]);
}

int wait_for_input(char const *argv[], int sockFd) {
    char buffer[16];
    char fileBuffer[BUF_SIZE];
    size_t numExec = 0;
    printf("Enter number of executable(s): ");
    scanf("%zu", &numExec);
    printf("%zu executable(s).\nEnter executable name: ", numExec);
    for (size_t i = 0; i < numExec; i++) {
        scanf("%s", buffer);
	printf("You entered: %s\n", buffer);
	if (access(buffer, X_OK) == 0) {
	    //execute(buffer);
	    //printf("executing %s\n", buffer);
            FILE *f = fopen(buffer, "r");
            write_binary_data(f, sockFd, fileBuffer);
	} else {
	    print_usage(argv);
	}
	printf("Enter executable name: ");
    }
    handleData();
    return 0;
}

int setUpClient(char *addr, char *port) {
    int s;
    int socket_fd = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0);
    int p = 1;
    s = setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, (char *) &p,
      sizeof(p));
    struct addrinfo hints, *result;
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    s = getaddrinfo(addr, port, &hints, &result);
    if (s != 0) {
        fprintf(stderr, "failed to find %s at port %s\n", addr, port);
        return -1;
    }
    if(connect(socket_fd, result->ai_addr, result->ai_addrlen) == -1) {
        if(errno != EINPROGRESS){
            perror("connect");
            return -1;
        }
        int i = 0;
        while (i++ < CONNECTION_ATTEMPTS_BEFORE_GIVING_UP){
            int checkVal;
            struct timeval tv; tv.tv_sec = 1; tv.tv_usec = 0;
            fd_set rfd; FD_ZERO(&rfd); FD_SET(socket_fd, &rfd);
            checkVal =select(socket_fd + 1, NULL, &rfd, NULL, &tv);
            fprintf(stdout, "checkVal is %d wit hsock %d\n", checkVal,
                    socket_fd);
            if (checkVal == -1) { return -1;}
            if (checkVal > 0) { break; }
            if (i == CONNECTION_ATTEMPTS_BEFORE_GIVING_UP){
                fprintf(stderr, "failed to find connection\n");
                return -1;
            }
        }
    }
    fprintf(stdout, "Found connection on fd %d\n", socket_fd);
    return socket_fd;
}

int cleanUpClient(int socket) {
    shutdown(socket, SHUT_WR);
    close(socket);
    return 0;
}

ssize_t my_write(int socket, void *buffer, size_t count) {
    size_t c = count;
    int y = 0;
    do {
        y = write(socket, buffer, count);
        if (y != -1) {
            buffer += y;
            count -= y;
        }
    } while ((count > 0 && 0 < y) || (y == -1 && errno == EINTR));
    if (y == -1 && errno != EINTR) {
        return -1;
    }
    return c - count;
}

void write_binary_data(FILE *f, int sockFd, char *buffer) {
    size_t s = 1;
    while (s != 0) {
        buffer[0] = '\0';
        s = fread(buffer, 1, BUF_SIZE, f);
        if (s != 0) {
            my_write(sockFd, buffer, s);
        }
    }
}

