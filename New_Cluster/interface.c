#include "interface.h"

int interface_main(int argc, char **argv) {

    int sockFd = socket(AF_INET, SOCK_STREAM, 0);
    struct addrinfo hints, *result;
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    int ga = getaddrinfo(argv[1], "9999", &hints, &result);
    if (ga != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(ga));
        freeaddrinfo(result);
        exit(1);
    }
    if(connect(sockFd, result->ai_addr, result->ai_addrlen) == -1){
        perror("connect");
        freeaddrinfo(result);
        exit(2);
    }
    char *buffer = malloc(BUF_SIZE);
    strcpy(buffer, "INTERFACE_PUT");
    if (access(argv[2], R_OK) != 0) {
        free(buffer);
        freeaddrinfo(result);
        exit(1);
    }

    send_request(sockFd, buffer, argv[2]);
    FILE *f = fopen(argv[2], "r");
    size_t s = get_file_size(argv[2]);
    my_write(sockFd, (void *)&s, sizeof(size_t));
    write_binary_data(f, sockFd, buffer);

    shutdown(sockFd, SHUT_WR);
    if (!check_ok(sockFd, buffer)) {
        freeaddrinfo(result);
        exit(1);
    }

    size_t dataSize;
    my_read(sockFd, &dataSize, 8);
    print_binary_data(f, sockFd, buffer, dataSize);

    free(buffer);
    freeaddrinfo(result);
    return 0;
}

void send_request(int sockFd, char *buffer, char *local) {
    strcat(buffer, " ");
    strcat(buffer, local);
    strcat(buffer, "\n");
    my_write(sockFd, buffer, strlen(buffer));
}

size_t get_file_size(char *filename) {
    struct stat buf;
    stat(filename, &buf);
    return buf.st_size;
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

bool check_ok(int sockFd, char *buffer) {
    buffer[0] = '\0';
    size_t s = my_read(sockFd, buffer, 1);
    buffer[s] = '\0';
    if (buffer[0] == 'O') { // OK
        s = my_read(sockFd, buffer, 2);
    } else { // ERROR
        s = my_read(sockFd, buffer, 5);
        buffer[0] = '\0';
        s = my_read(sockFd, buffer, BUF_SIZE);
        buffer[s] = '\0';
        printf("%s", buffer);
        free(buffer);
        return false;
    }
    return true;
}

void print_binary_data(FILE *f, int sockFd, char *buffer, size_t dataSize) {
    size_t s = 1;
    size_t size = 0;
    while (s != 0) {
        buffer[0] = '\0';
        s = my_read(sockFd, buffer, BUF_SIZE);
        size += s;
        if (s != 0) {
            write(1, buffer, s);
        }
    }
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

ssize_t my_read(int socket, void *buffer, size_t count) {
    size_t c = count;
    int y = 0;
    do {
        y = read(socket, buffer, count);
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
