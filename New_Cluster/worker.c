#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdbool.h>
#include <pthread.h>
#include "errno.h"

#include "queue.h"

#define PUT_REQUEST "PUT"
#define PUT_REQUEST_SIZE (strlen(PUT_REQUEST))
#define NUM_THREAD (5)
#define MAX_CONCURRENT_TASK (15)
#define OUTPUT_FILE_FORMAT "_OUTPUT.OUTPUT"
#define BAD_EXEC_MESSAGE "Bad executable: "
#define FILE_BUFFER_SIZE (4096)
#define LINE_BUFFER_SIZE (32)

typedef struct task_t{
    char* input_filename;
    char* output_filename;
    int output_fd;
} task_t;

void* task_copy_constructor(void* elem) {
    task_t* new_task = malloc(sizeof(task_t));
    task_t* old_task = (task_t*)elem;
    new_task->input_filename = old_task->input_filename; // shallow copy
    new_task->output_filename = old_task->output_filename; // shallow copy
    new_task->output_fd = old_task->output_fd;
    return new_task;
}

void task_destructor(void* elem) {
    task_t* old_task = (task_t*)elem;
    free(old_task->input_filename);
    free(old_task->output_filename);
    free(old_task);
}

int worker_main(char* host, char* port);
void* sending(void* nothing);
void* tasking(void* nothing);
int create_output_file(char* input_filename, char** output_filename);
void read_file(char* local_path);
char* get_filename(char* header, size_t header_size, size_t verb_size);
ssize_t write_size_to_socket(int socket_fd, size_t size);
ssize_t write_file_to_socket(int socket_fd, int local_file_fd, size_t local_file_size);
ssize_t write_to_socket(int socket_fd, char* buffer, ssize_t count);
ssize_t read_file_from_socket(int socket_fd, int local_file_fd, size_t count);
ssize_t read_from_socket(int socket_fd, char* buffer, ssize_t count);
ssize_t read_size_from_socket(int socket_fd, size_t* size);
ssize_t read_line_from_socket(int socket_fd, char** buffer);
void allocate_buffer(char** buffer, size_t size);
int setup_client(char* host, char* port);
char* create_header(char* filename);

static int socket_fd;

static queue* task_queue;
static queue* finished_task_queue;

int main(int argc, char const *argv[]) {
    if (argc < 3) {
        printf("USAGE: ./worker host port\n");
        return 1;
    }
    worker_main((char*)argv[1], (char*)argv[2]);
    return 0;
}

int worker_main(char* host, char* port) {

    socket_fd = setup_client(host, port);
    task_queue = queue_create(MAX_CONCURRENT_TASK, task_copy_constructor, task_destructor); // set the limit in case of malicious overloading
    finished_task_queue = queue_create(-1, task_copy_constructor, task_destructor);

    pthread_t tasking_threads[NUM_THREAD];
    pthread_t sending_thread;
    for (int i=0; i < NUM_THREAD; i++) {
        pthread_create(&tasking_threads[i], NULL, tasking, NULL);
    }
    pthread_create(&sending_thread, NULL, sending, NULL);

    while(1) {
        char* request = NULL;

        ssize_t byteRead = read_line_from_socket(socket_fd, &request);
        if (byteRead == -1) {
            perror(NULL);
            exit(-1);
        }
        if (byteRead == 0) {
            printf("Master socket is closed\n");
            break; // break on master failure
        }
        if (strncmp(request, PUT_REQUEST, PUT_REQUEST_SIZE) != 0) {
            printf("Bad request from master\n");
            continue;
        }
        printf("Received request: %s", request);
        char* filename = get_filename(request, byteRead, PUT_REQUEST_SIZE);
        if (filename == NULL) {
            printf("Bad request\n");
            continue;
        }

        read_file(filename);
        char* output_filename;
        int output_fd = create_output_file(filename, &output_filename);
        task_t new_task = (task_t){filename, output_filename, output_fd};
        queue_push(task_queue, &new_task);
    }

    // clean up

    task_t end = (task_t){NULL, NULL, 0};
    for (int i=0; i < NUM_THREAD; i++) {
        queue_push(task_queue, &end);
    }
    queue_push(finished_task_queue, &end);
    for (int i=0; i < NUM_THREAD; i++) {
        pthread_join(tasking_threads[i], NULL);
    }
    pthread_join(sending_thread, NULL);

    queue_destroy(task_queue);
    queue_destroy(finished_task_queue);
    close(socket_fd);

    return 0;
}

void* sending(void* nothing) {
    (void) nothing;
    while (1) {
        task_t* task = queue_pull(finished_task_queue);
        if (task->input_filename == NULL) {
            break;
        }
        printf("Sending back data output of %s\n", task->input_filename);

        char* buffer = create_header(task->input_filename);
        if (write_to_socket(socket_fd, buffer, strlen(buffer)) == -1) {
            perror(NULL);
            printf("Exiting thread due to error\n");
            break;
        }

        int local_file_fd = task->output_fd;
        size_t file_size = lseek(local_file_fd, 0, SEEK_END);
        lseek(local_file_fd, 0, SEEK_SET);

        if (write_size_to_socket(socket_fd, file_size) == -1) {
            perror(NULL);
            printf("Exiting thread due to error\n");
            break;
        }
        if (write_file_to_socket(socket_fd, local_file_fd, file_size) == -1) {
            perror(NULL);
            printf("Exiting thread due to error\n");
            break;
        }
        close(local_file_fd);
    }
    return NULL;
}

void* tasking(void* nothing) {
    (void) nothing;
    while(1) {
        task_t* task = queue_pull(task_queue);
        if (task->input_filename == NULL) {
            break;
        }
        char buffer[1024];
        char* executable_name = task->input_filename;
        sprintf(buffer, "./%s >%s 2>&1", executable_name, task->output_filename);
        printf("Executing file: %s\n", executable_name);
        if (system(buffer) == -1) {
            perror(NULL);
            printf("Execution failed\n");
            write(task->output_fd, BAD_EXEC_MESSAGE, strlen(BAD_EXEC_MESSAGE));
            write(task->output_fd, executable_name, strlen(executable_name) + 1); // writing the null byte
        }
        printf("Finished executing\n");
        queue_push(finished_task_queue, task);
    }
    return NULL;
}

// FILE MANAGEMENT CODE

char* create_header(char* filename) {
    char* buffer;
    buffer = malloc(PUT_REQUEST_SIZE + strlen(filename) + 3);
    sprintf(buffer, "%s %s\n", PUT_REQUEST, filename);
    return buffer;
}

int create_output_file(char* input_filename, char** output_filename) { // sets output_filename

    char* buffer = malloc(strlen(input_filename) + strlen(OUTPUT_FILE_FORMAT) + 1);
    sprintf(buffer, "%s%s", input_filename, OUTPUT_FILE_FORMAT);
    *output_filename = buffer;
    int local_file_fd = open(buffer, O_CREAT | O_RDWR | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH);
    return local_file_fd;
}

void read_file(char* local_path) {

    size_t file_size;
    if (read_size_from_socket(socket_fd, &file_size) == -1) {
        perror(NULL);
        exit(-1);
    }
    int local_file_fd = open(local_path, O_CREAT | O_RDWR | O_TRUNC, S_IRUSR | S_IWUSR | S_IXUSR | S_IRGRP | S_IWGRP | S_IXGRP | S_IROTH | S_IWOTH | S_IXOTH);
    if (local_file_fd == -1) {
        perror(NULL);
        exit(-1);
    }
    if (read_file_from_socket(socket_fd, local_file_fd, file_size) < 0) {
        perror(NULL);
        exit(-1);
    }
    close(local_file_fd);
}

char* get_filename(char* header, size_t header_size, size_t verb_size) {

    ssize_t name_size = header_size - verb_size - 2;
    if (name_size < 0) {
        return NULL;
    }
    char* filename = calloc(1, name_size + 1);
    filename = strncpy(filename, header + verb_size + 1, name_size);
    return filename;
}

// NETWORKING CODE

ssize_t write_size_to_socket(int socket_fd, size_t size) {
    return write_to_socket(socket_fd, (char*)&size, sizeof(size));
}

ssize_t write_file_to_socket(int socket_fd, int local_file_fd, size_t local_file_size) {

    size_t totalByteWrote = 0;
    ssize_t byteWrote;

    char* buffer = malloc(sizeof(char)*FILE_BUFFER_SIZE);
    size_t remain_size = local_file_size;
    size_t count;

    while (remain_size > 0) {

        if (remain_size >= FILE_BUFFER_SIZE) {
            read(local_file_fd, buffer, FILE_BUFFER_SIZE);
            count = FILE_BUFFER_SIZE;
        } else {
            read(local_file_fd, buffer, remain_size);
            count = remain_size;
        }

        byteWrote = write_to_socket(socket_fd, buffer, count);
        if (byteWrote == -1) {
            return -1;
        }
        totalByteWrote += byteWrote;
        remain_size -= byteWrote;
    }

    free(buffer);
    return totalByteWrote;
}

ssize_t write_to_socket(int socket_fd, char* buffer, ssize_t count) {

    ssize_t totalByteWrote = 0;
    ssize_t byteWrote;

    while(1) {
        byteWrote = write(socket_fd, buffer + totalByteWrote, count - totalByteWrote);
        if (byteWrote == 0)
            return totalByteWrote;
        if (byteWrote == -1) {
            if (errno != EINTR)
                return -1;
            continue;
        }
        totalByteWrote += byteWrote;
        if (totalByteWrote >= count) {
            break;
        }
    }
    return totalByteWrote;
}

ssize_t read_file_from_socket(int socket_fd, int local_file_fd, size_t count) {

    size_t remain_size = count;
    size_t totalByteRead = 0;
    ssize_t byteRead;

    char* buffer = malloc(sizeof(char)*FILE_BUFFER_SIZE);
    while (1) {

        if (remain_size >= FILE_BUFFER_SIZE)
            byteRead = read_from_socket(socket_fd, buffer, FILE_BUFFER_SIZE);
        else
            byteRead = read_from_socket(socket_fd, buffer, remain_size);

        if (byteRead == -1) {
            return -1;
        }
        totalByteRead += byteRead;
        remain_size -= byteRead;
        write(local_file_fd, buffer, byteRead);
        if (remain_size == 0) {
            break;
        }
    }

    free(buffer);
    return totalByteRead;
}

ssize_t read_from_socket(int socket_fd, char* buffer, ssize_t count) {

    ssize_t totalByteRead = 0;
    ssize_t byteRead;

    while(1) {
        byteRead = read(socket_fd, buffer + totalByteRead, count - totalByteRead);
        if (byteRead == 0)
            return totalByteRead;
        if (byteRead == -1) {
            if (errno != EINTR)
                return -1;
            continue;
        }
        totalByteRead += byteRead;
        if (totalByteRead >= count) {
            break;
        }
    }
    return totalByteRead;
}

ssize_t read_size_from_socket(int socket_fd, size_t* size) {
    size_t buffer;
    ssize_t byteRead = read_from_socket(socket_fd, (char*)&buffer, sizeof(buffer));
    *size = buffer;
    return byteRead;
}

ssize_t read_line_from_socket(int socket_fd, char** buffer) {

    ssize_t size = LINE_BUFFER_SIZE;
    allocate_buffer(buffer, size);
    ssize_t totalByteRead = 0;
    ssize_t byteRead;

    while(1) {
        byteRead = read(socket_fd, (*buffer) + totalByteRead, 1);
        if (byteRead == 0)
            break;
        if (byteRead == -1) {
            if (errno != EINTR)
                return -1;
            continue;
        }
        totalByteRead += byteRead;
        if ((*buffer)[totalByteRead - 1] == '\n')
            break;
        if (totalByteRead == size) {
            size += LINE_BUFFER_SIZE;
            allocate_buffer(buffer, size);
        }
    }
    return totalByteRead;
}

void allocate_buffer(char** buffer, size_t size) {
    *buffer = realloc(*buffer, size);
}

int setup_client(char* host, char* port) {

    int socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    struct addrinfo hints;
    struct addrinfo *result;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    int status = getaddrinfo(host, port, &hints, &result);
    if (status != 0) {
        fprintf(stderr, "%s", gai_strerror(status));
        exit(-1);
    }
    if (connect(socket_fd, result->ai_addr, result->ai_addrlen) == -1) {
        perror(NULL);
        exit(-1);
    }

    freeaddrinfo(result);
    return socket_fd;
}
