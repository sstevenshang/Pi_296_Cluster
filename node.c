#include "node.h"

pthread_mutex_t node_list_m = PTHREAD_MUTEX_INITIALIZER;

void addNode(int socketFd, char *address, node **head) {

    node *newNode = malloc(sizeof(node));

    newNode->socket_fd = socketFd;
    newNode->alive = 1;
    newNode->cur_load = 0;
    newNode->taskNo = 0;
    newNode->taskPos = 0;
    newNode->next = NULL;
    newNode->address = address;
    newNode->last_beat_received_time = 0;
    newNode->bufSize = 4096;
    newNode->bufPos = 0;
    newNode->buf = (void*)malloc(newNode->bufSize);
    newNode->bufWIP = 0;
    newNode->num_of_task = 0;
    newNode->task_list = malloc(sizeof(task)*MAX_TASKS_PER_NODE);

    pthread_mutex_lock(&node_list_m);

    if(*head == NULL) {
        *head = newNode;
        pthread_mutex_unlock(&node_list_m);
        return;
    }

    node* temp = *head;
    while(get_next(temp) != NULL) {
        temp = get_next(temp);
    }
    set_next(temp, newNode);
}

void removeNode(node *oldNode, node **head) {
    pthread_mutex_lock(&node_list_m);

    if (*head == oldNode) {
        *head = oldNode->next;
        pthread_mutex_unlock(&node_list_m);
        cleanNode(oldNode);
        return;
    }

    node *holder = *head;
    while (holder != NULL) {
        if (holder->next == oldNode) {
            holder->next = oldNode->next;
            pthread_mutex_unlock(&node_list_m);
            cleanNode(oldNode);
            return;
        }
        holder = get_next(holder);
    }
    pthread_mutex_unlock(&node_list_m);
    fprintf(stderr, "You tried removing an invalid node address: %p\n",oldNode);
}

void cleanNode(node* to_free) {
  free(to_free->address);
  free(to_free->task_list);
  free(to_free);
}

int is_alive(node* cur) {
  if (!cur)
    return -1;
  pthread_mutex_lock(&node_list_m);
  int alive = cur->alive;
  pthread_mutex_unlock(&node_list_m);
  return alive;
}

void free_all_nodes(node *head) {
  node* iter = head;
  while (iter) {
    node* temp = get_next(iter);
    cleanNode(iter);
    iter = temp;
  }
}

node* searchNodeByAddr(char* beat_addr, node *head) {
  node* temp = head;
  while(temp != NULL) {
    if (strcmp(beat_addr, get_address(temp)) == 0) {
      return temp;
    }
    temp = get_next(temp);
  }
  return NULL;
}

node* get_next(node* curr) {
  if (!curr)
    return NULL;
  pthread_mutex_lock(&node_list_m);
  node* next = curr->next;
  pthread_mutex_unlock(&node_list_m);
  return next;
}

void set_next(node* prev, node* next) {
  pthread_mutex_lock(&node_list_m);
  prev->next = next;
  pthread_mutex_unlock(&node_list_m);
}

void set_last_beat_received_time(node* curr, double time) {
  pthread_mutex_lock(&node_list_m);
  curr->last_beat_received_time = time;
  pthread_mutex_unlock(&node_list_m);
}

void set_load(node* curr, double cpu_load) {
  pthread_mutex_lock(&node_list_m);
  curr->cur_load = cpu_load;
  pthread_mutex_unlock(&node_list_m);
}

double get_load(node* curr) {
  pthread_mutex_lock(&node_list_m);
  double load = curr->cur_load;
  pthread_mutex_unlock(&node_list_m);
  return load;
}

void set_alive(node* curr, int alive) {
  pthread_mutex_lock(&node_list_m);
  curr->alive = alive;
  pthread_mutex_unlock(&node_list_m);
}

double get_last_heartbeat_time(node* curr) {
  pthread_mutex_lock(&node_list_m);
  double last_heatbeat_time = curr->last_beat_received_time;
  pthread_mutex_unlock(&node_list_m);
  return last_heatbeat_time;
}

char* get_address(node* curr) {
  pthread_mutex_lock(&node_list_m);
  char* address = curr->address;
  pthread_mutex_unlock(&node_list_m);
  return address;
}

int get_num_of_task(node* cur) {
  pthread_mutex_lock(&node_list_m);
  int num_of_task = cur->num_of_task;
  pthread_mutex_unlock(&node_list_m);
  return num_of_task;
}

void set_num_of_task(node* cur, int num_of_task) {
  pthread_mutex_lock(&node_list_m);
  cur->num_of_task = num_of_task;
  pthread_mutex_unlock(&node_list_m);
}

void destroy_mutex() {
  pthread_mutex_destroy(&node_list_m);
}

void free_task(task* elem) {
    free(elem->file_name);
    free(elem);
}
