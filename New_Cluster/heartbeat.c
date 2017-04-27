
// HEART_BEAT CODE
int setUpUDPServer() {
    int socket_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (socket_fd < 0) {
        perror("FAILED: unable to create socket");
        return -1;
    }

    struct sockaddr_in serverAddr;
    memset((char*)&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    serverAddr.sin_port = htons(9111);

    int status = bind(socket_fd, (struct sockaddr*) &serverAddr,
            sizeof(serverAddr));
    if (status < 0) {
        perror("FAILED: unable to bind socket");
        return -1;
    }

    int optval = 1;
    setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));
    setsockopt(socket_fd, SOL_SOCKET, SO_REUSEPORT, &optval, sizeof(optval));

    return socket_fd;
}

void* listenToHeartbeat(void* keepalive) {
    struct sockaddr_in clientAddr;
    memset((char*)&clientAddr, 0, sizeof(clientAddr));
    socklen_t addrlen = sizeof(clientAddr);
    int byte_received = 0;

    int socket_fd = setUpUDPServer();
    int keep_listenning = *((int*)keepalive);

    while(keep_listenning) {
        char buffer[50];
        byte_received = recvfrom(socket_fd, &buffer, sizeof(buffer), 0,
                (struct sockaddr*)&clientAddr, &addrlen);
        double client_usage = atof(buffer);
        if (byte_received < 0) {
            perror("FAILED: failed to receive from client");
        } else {
            char* beat_addr = inet_ntoa(clientAddr.sin_addr);
            printf("SUCCESS: received \"%f\" from %s\n", client_usage,
                    beat_addr);
            reportHeartbeat(beat_addr, client_usage);
        }
    }
    close(socket_fd);
    return NULL;
}

void reportHeartbeat(char* beat_addr, double client_usage) {
    double time_received = getTime();
    node *reported_node = searchNodeByAddr(beat_addr, workerList);
    if (reported_node) {
        set_last_beat_received_time(reported_node, time_received);
        set_load(reported_node, client_usage);
    }
}

double getTime() {
    struct timespec t;
    clock_gettime(CLOCK_MONOTONIC, &t);
    return t.tv_sec + 1e-9 * t.tv_nsec;
}

void* keepNodesInCheck(void* load) {
    (void) load;  //Type casting to void to circumvent compiler warnings.
    double cur_time;
    while(runningM) {
        node* cur = workerList; // head
        while (cur != NULL) {
            cur_time = getTime();
            if (get_last_heartbeat_time(cur) != 0) {
                if (cur_time - get_last_heartbeat_time(cur) > 3.0) {
                    printf("node %s is dead\n", get_address(cur));
                    //Mark task as dead
                    set_alive(cur, 0);
                    //Any tasks that were running on the node
                    recover_tasks(cur);
                    break;
                }
            }
            cur = get_next(cur);
        }
        sleep(2);
    }
    return NULL;
}
