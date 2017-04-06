#include "heartbeat.h"
#include "master.h"
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>

#define BUFSIZE 1024
#define SERVER_PORT 1153
#define SERVER_IP4 "127.0.0.1"

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

int setUpUDPServer() {

	int socket_fd = socket(AF_INET, SOCK_DGRAM, 0);
	if (socket_fd < 0) {
		perror("FAILED: unable to create socket");
		return -1;
	}
	struct sockaddr_in serverAddr;
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(SERVER_PORT);
	serverAddr.sin_addr.s_addr = inet_addr(SERVER_IP4);
	memset(serverAddr.sin_zero, 0, sizeof(serverAddr.sin_zero));

	int status = bind(socket_fd, (struct  sockaddr*) &serverAddr, sizeof(serverAddr));
	if (status < 0) {
		perror("FAILED: unable to bind socket");
		return -1;
	}

	return socket_fd;
}


void heartbeat(char* destinationAddr, char* destinationPort, int* alive) {
	int socket_fd = setUpUDPClient();
	while (*alive) {
		sleep(3);
		while (sendHeartbeat(socket_fd, destinationAddr, destinationPort) == -1) {
			printf("Failed: failed to send heartbeat");
		}
	}
	cleanupUDPSocket(socket_fd);
}

int sendHeartbeat(int socket_fd, char* destinationAddr, char* destinationPort) {

	// char* message = "BEAT";
	double cpu_usage = get_local_usage();
	// size_t length = strlen(message) + 1;
	size_t length = sizeof(cpu_usage);

	struct sockaddr_in serverAddr;

	serverAddr.sin_family = AF_INET;
	serverAddr.sin_addr.s_addr = inet_addr(destinationAddr);
	serverAddr.sin_port = inet_addr(destinationPort);
	memset(serverAddr.sin_zero, 0, sizeof(serverAddr.sin_zero));

	socklen_t dest_len = sizeof(serverAddr);

	int status = sendto(socket_fd, message, length, 0, (struct sockaddr*) &serverAddr, dest_len);

	if (status < 0) {
		perror("FAILED: unable to send message to server");
		return -1;
	}

	return 0;
}

void listenToHeartbeat(int* stethoscope) {

	// unsigned char buffer[BUFSIZE];
	double client_usage;
	struct sockaddr_in clientAddr;
	socklen_t addrlen = sizeof(clientAddr);
	int byte_received = 0;
	int socket_fd = setUpUDPClient();

	while(*stethoscope) {

		byte_received = recvfrom(socket_fd, &client_usage, sizeof(client_usage), 0, (struct sockaddr*)&clientAddr, &addrlen);
		if (byte_received < 0) {
			perror("FAILED: failed to receive from client");
		} else {
			char* beat_addr = inet_ntoa(clientAddr.sin_addr);
			//char* beat_port = inet_ntoa(clientAddr.sin_port);
			reportHeartbeat(beat_addr);
			printf("SUCCESS: received \"%s\" from %s\n", buffer, beat_addr/*, beat_port*/);
		}
	}
	cleanupUDPSocket(socket_fd);
}
