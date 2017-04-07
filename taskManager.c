#include "taskManager.h"


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

void handleTaskOne(node* task){
  
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
