#include "networkManager.h"

int setUpClient(node_id* my_id) {

	char* addr = my_id->addr;
	char* port = my_id->port;

	int status;
	struct addrinfo hints, *result;
  	memset(&hints, 0, sizeof(struct addrinfo));

  	int socket_fd = socket(AF_INET, SOCK_STREAM, 0);

  	hints.ai_family = AF_INET;
  	hints.ai_socktype = SOCK_STREAM;

  	status = getaddrinfo(addr, port, &hints, &result);
  	if (status != 0) {
  		fprintf(stderr, "FAILED: unable to find %s at port %s\n", addr, port);
  		return -1;
  	}

  	status = connect(socket_fd, result->ai_addr, result->ai_addrlen);
  	if (status == -1) {
  		fprintf(stderr, "FAILED: socket %d failed to connect (%s)\n", socket_fd, strerror(errno));
  		return -1;
  	}

	fprintf(stdout, "SUCCESS: scoket %d connected\n", socket_fd);
  	return socket_fd;
}

// TODO
int setUpServer(node_id* my_id) {

	char* addr = my_id->addr;
	char* port = my_id->port;

	int status;
  	struct addrinfo hints, *result;
  	memset(&hints, 0, sizeof(struct addrinfo));

  	int socket_fd = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0);

	hints.ai_family = AF_INET;
  	hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;
  	status = getaddrinfo(addr, port, &hints, &result);
  	if (status != 0) {
  		fprintf(stderr, "FAILED: unable to find %s at port %s\n", addr, port);
  		return -1;
  	}

	int optval = 1;
  	setsockopt(socket_fd, SOL_SOCKET, SO_REUSEPORT, &optval, sizeof(optval));
	status = bind(socket_fd, result->ai_addr, result->ai_addrlen);
	if (status != 0) {
  		fprintf(stderr, "FAILED: failed to bind to socket %d (%s)\n", socket_fd, strerror(errno));
  		return -1;
	}

  	status = listen(socket_fd, 10);
  	if (status != 0) {
  		fprintf(stderr, "FAILED: failed to listen to socket %d (%s)\n", socket_fd, strerror(errno));
    	return -1;
  	}

	struct sockaddr_in *result_addr = (struct sockaddr_in*)result->ai_addr;
	my_id->master_socket = socket_fd;
	return socket_fd;
}


