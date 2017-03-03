#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <signal.h>
#include <errno.h>
void pipecatcher(int signum){
    fprintf(stdout, "hit a sigpipe\n");
}

int main(int argc, char **argv)
{
    signal(SIGPIPE, pipecatcher);
    int s;
    int sock_fd = socket(AF_INET, SOCK_STREAM, 0);

    struct addrinfo hints, *result;
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    s = getaddrinfo(NULL, "9001", &hints, &result);
    if (s != 0) {
            fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(s));
            exit(1);
    }
    int optval = 1;
    setsockopt(sock_fd, SOL_SOCKET, SO_REUSEPORT, &optval, sizeof(optval));
    if (bind(sock_fd, result->ai_addr, result->ai_addrlen) != 0) {
        perror("bind()");
        exit(1);
    }

    if (listen(sock_fd, 10) != 0) {
        perror("listen()");
        exit(1);
    }

    struct sockaddr_in *result_addr = (struct sockaddr_in *) result->ai_addr;
    printf("Listening on file descriptor %d, port %d\n", sock_fd, ntohs(result_addr->sin_port));

    printf("Waiting for connection...\n");
    int client_fd = accept(sock_fd, NULL, NULL);
    printf("Connection made: client_fd=%d\n", client_fd);
//write(sock_fd, "THIS IS TEST", 13);
    char temp[100];
    int temp2 = read(client_fd, temp, 99);
    temp[temp2] = '\0'; fprintf(stdout, "temp2: %d temp is %s\n", temp2, temp);
    if(temp2 == -1){ fprintf(stdout, "error gotten by temp2: %s\n", strerror(errno));}

    FILE* exFile = fopen("./hi.out", "rb");
    if(exFile == NULL){ perror("Failed opening ./hi.out"); exit(1);}
    char resp[1000];
    int len = read(fileno(exFile), resp, 999);
    resp[len] = '\0';
    fprintf(stdout, "len = %d in len is : %s\n", len, resp);
    while(len != -1 && len != 0){
//fprintf(stdout, "len is %d\n", len);
                write(client_fd, resp, len);
  // 	puts("hit2");
               len =  read(fileno(exFile), resp, 999);
		read(client_fd, (void*)(0), 0);
//puts("hit");
    }
    shutdown(sock_fd, SHUT_WR);
    fprintf(stdout, "shutdown socket\n");
    return 0;
}

