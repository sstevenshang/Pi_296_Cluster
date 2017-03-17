#include "master.h"

/* MASTER FUNCTIONS */
int runningM = 0;
int clientIncomingFd = -1;
char* defaultMasterPort = "9001";
int master_main() {

	// Spawn thread for heartbeat
/*	pthread_t heartbeat_thread;
	pthread_t stethoscope_thread;

	int listen = 1;

	pthread_create(&heartbeat_thread, NULL, listenToHeartbeatThread, &listen);
	pthread_create(&stethoscope_thread, NULL, updateNodeStatusThread, &listen);

	updateNodeStatusThread(&listen);

	while(1) {
		// Wait for task input
		// Distribute task
		// Spawn thread to send request
	}

	pthread_join(heartbeat_thread, NULL);
	pthread_join(stethoscope_thread, NULL);
*/
        setUpMaster(defaultMasterPort);
        runningM = 1;
        while(runningM == 1){
        
        }
        cleanUpMaster(clientIncomingFd);
	return 0;
}

int setUpMaster(char* port){
  int s;
  int socket_fd = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0);
  struct addrinfo hints, *result;
  memset(&hints, 0, sizeof(struct addrinfo));
  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = AI_PASSIVE;
  s = getaddrinfo(NULL, port, &hints, &result);
  if(s != 0){
    fprintf(stderr, "failed to find at port %s\n", port);
    return -1;
  }
  int optval = 1;
  setsockopt(socket_fd, SOL_SOCKET, SO_REUSEPORT, &optval, sizeof(optval));
  if(bind(socket_fd, result->ai_addr, result->ai_addrlen) != 0) {
      perror("bind()");
      return -1;
  }
  if(listen(socket_fd, 10) != 0) {
      perror("listen()");
      return -1;
    }
  //struct sockaddr_in *result_addr = (struct sockaddr_in*)result->ai_addr;
  clientIncomingFd = socket_fd;
  return 1;
}

int addAnyIncomingConnections(){
  int client_fd = accept(clientIncomingFd, NULL, NULL);
  if(client_fd != -1){  //found a connection
    fprintf(stdout, "got something\n");
    addNode(client_fd);
    return client_fd;
  }
  //fprintf(stdout, "no connections yet\n");
    return client_fd;
}

int cleanUpMaster(int socket){
  shutdown(socket, SHUT_WR);
  return 0;
}

int getFdForReadFile(char* name){
  FILE * read_fd = fopen(name, "rb");
  if(read_fd == NULL){
    fprintf(stderr, "Error opening file %s\n", name);
    return -1; 
  }
  return fileno(read_fd);
}

int sendBinaryFile(int socket, char* name){
  int file = getFdForReadFile(name);
  if(file == -1){ return -1;}
  int bufSize = 2048;
  char buf[bufSize]; // completely abritray, could be up to 65535 (2^16 -1)
  int len = read(file, buf, bufSize); //holds if no data in pipeline
  while(len != 0){ //len read something
    if(len == -1){
      fprintf(stderr, "some error with reading from the fd %d, check for SIGPIPE\n", file);
      return 0;
    }
    int s = write(socket, buf, len);
    if(s == -1){ close(file); return 0;}
    while(s != len){ // as long as write hasn't written all bytes
      int k = write(socket, buf + s, len - s);
      if(k == -1){ close(file); return 0;}
      s += k;
    }
    s = read(socket, NULL, 0); // ingnoring, putting more data
    len = read(file, buf, bufSize);
  }
  if(len == -1){
    fprintf(stderr, "some error with reading from the fd %d, check for SIGPIPE\n", file);
    return 0;
  }
  close(file);
  return 0;
}
/*
void reportHeartbeat(char* beat_addr) {
	double time_received = getTime();
	node* temp = traverseNodes(beat_addr);
	temp->last_beat_received_time = time_received;
}

void* listenToHeartbeatThread(void* listen) {
	listenToHeartbeat((int*)listen);
	return NULL;
}


void* updateNodeStatusThread(void* listen) {

	resetBeats();
	double time;
	while(*((int*)listen)) {
		node* temp = head;
		for (size_t i=0; i<node_counts; i++) {
			if (temp->alive) {
				time = getTime();
				if ((time - temp->last_beat_received_time) > 9) {
					temp->alive = IS_DEAD;
				}
				temp = temp->next;
			}
		}
		sleep(2);
	}
	return NULL;
}

void resetBeats() {
		node* temp = head;
		for (size_t i=0; i<node_counts; i++) {
			double time = getTime();
			temp->last_beat_received_time = time;
			temp = temp->next;
		}
}

double getTime() {
  struct timespec t;
  clock_gettime(CLOCK_MONOTONIC, &t);
  return t.tv_sec + 1e-9 * t.tv_nsec;
}
*/
