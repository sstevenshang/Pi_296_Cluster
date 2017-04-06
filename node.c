#include "node.h"

node* head = NULL;
node* lastInList = NULL;

void addNode(int socketFd, char* address){
  node* newNode = (node*)malloc(sizeof(node));
  newNode->socket_fd = socketFd;
  newNode->alive = 1;
  newNode->cur_load = 0;
  newNode->taskNo = 0;
  newNode->taskPos = 0;
  newNode->next = NULL;
  newNode->address = address;
  newNode->last_beat_received_time = 0;
  if(head == NULL){
    head = newNode;
    lastInList = newNode;
  } else {
    lastInList->next = newNode;
    lastInList = newNode;
  }
}

void removeNode(node* oldNode){

  if(head == oldNode){
    head = oldNode->next;
    cleanNode(oldNode);
  }
  node* holder = head;
  
  while(holder != NULL){
    if(holder->next == oldNode){
      holder->next = oldNode->next;
      cleanNode(oldNode);
      exit(0);
    }
    holder = holder->next;
  }
  fprintf(stderr, "You tried removing an invalid node address: %p\n", oldNode);
}

void cleanNode(node* to_free) {
  free(node->address);
  free(node);
} 

node* searchNodeByAddr(char* beat_addr) {
  node* temp = head;
  while(temp != NULL) {
    if (strcmp(beat_addr, temp->address) == 0) {
      return temp;
    }
    temp = temp->next;
  }
  return NULL;
}
