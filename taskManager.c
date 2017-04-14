#include "taskManager.h"

int messageBufSize = 4096;
char messageBuf[4096];

int readSocketIntoBuf(int socket, void* buf, int bufSize){
  ssize_t bytesRead = read(socket, buf, bufSize);
  if(bytesRead == -1){
    if(errno == EWOULDBLOCK){ return 0;}
    fprintf(stderr, "Invalid read from socket %d\n", socket);
    return -1;
  } else {
    return 1;
  }
}


void manageTask(node* head){
  if(head == NULL){return;}

  node* tmp = head;
  while(tmp != NULL){
//    puts("hitTask");
    if(tmp->taskNo == 0){ //no task assigned
      
    } else if(tmp->taskNo == 1){
      handleTaskOne(tmp);
    } else if(tmp->taskNo == 2){
      handleTaskTwo(tmp);
    } else if(tmp->taskNo == 3){
      handleTaskThree(tmp);
    } else if(tmp->taskNo == 4){
      handleTaskFour(tmp);
    } else if(tmp->taskNo == 5){
      handleTaskFive(tmp);
    } else if(tmp->taskNo == 6){
      handleTaskSix(tmp);
    } else {
      fprintf(stderr, "taskmanager got a node with weird taskno %d\n", tmp->taskNo);
    }


    tmp = tmp->next;
  }
}

int getMessageType(char* header){
  unsigned char headerByte = *header;
//  fprintf(stdout, "gotten headerbyte is %d\n", (int)headerByte);
  int type = 7;
  while(type != 0){
    if((headerByte & 0x80) == 0x80){
      if(((headerByte >> (7-type))& 0x01) == 0x01){
  //      fprintf(stdout, "in getMessageType, returning %d\n", type);
        return type;
      }
        return -1;
    }
    headerByte = headerByte << 1;
    type --;
  }
  return 0;

}

void handleTaskOne(node* task){
  int bytesRead = readSocketIntoBuf(task->socket_fd, messageBuf, messageBufSize);
  if (bytesRead == -1){
    return;
  } else if(bytesRead == 0){
    return;
  } else {
    
    int pos = task->taskPos;
    switch(pos){
      case 0:
	break;
      case 1:
	break;
      case 2:
	break;
    }


  }
  //expects a message RECEIVEME
  
  //sends RECEIVED
  //recieves READY
}

void handleTaskTwo(node* task){

}
void handleTaskThree(node* task){

}
void handleTaskFour(node* task){

}
void handleTaskFive(node* task){

}
void handleTaskSix(node* task){

}
