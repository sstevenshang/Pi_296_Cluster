#include "node.h"

void addNode(int socketFd, char *address, node **head) {
    node *newNode = (node *)malloc(sizeof(node));
    newNode->socket_fd = socketFd;
    newNode->alive = 1;
    newNode->cur_load = 0;
    newNode->taskNo = 0;
    newNode->taskPos = 0;
    newNode->next = NULL;
    newNode->address = address;
    newNode->last_beat_received_time = 0;
    if(*head == NULL){*head = newNode; return ;}
    node* tmp = *head;
    node* tmpNext = tmp->next;
    while(tmpNext != NULL){ tmpNext = tmpNext->next;}
    tmpNext = newNode;
    puts("hit");
}

void removeNode(node *oldNode, node *head) {
    if (head == oldNode) {
        head = oldNode->next;
        cleanNode(oldNode);
    }
    node *holder = head;

    while (holder != NULL) {
        if (holder->next == oldNode) {
            holder->next = oldNode->next;
            cleanNode(oldNode);
            // exit(0);
            return;
        }
        holder = holder->next;
    }
    fprintf(stderr, "You tried removing an invalid node address: %p\n",
        oldNode);
}

void cleanNode(node* to_free) {
  free(to_free->address);
  free(to_free);
}

void free_all_nodes(node *head) {
  node* iter = head;
  while (iter) {
    node* temp = iter->next;
    cleanNode(iter);
    iter = temp;
  }
}

node* searchNodeByAddr(char* beat_addr, node *head) {
  node* temp = head;
  while(temp != NULL) {
    if (strcmp(beat_addr, temp->address) == 0) {
      return temp;
    }
    temp = temp->next;
  }
  return NULL;
}
