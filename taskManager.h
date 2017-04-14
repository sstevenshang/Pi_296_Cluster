#ifndef _TASK_MANAGER_H_
#define _TASK_MANAGER_H_

#include "node.h"
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define READY "Ready for work."
#define RECEIVEME "Do you receive me?"
#define RECEIVED "Yes, I received you."
#define RUNNING "Running."
#define ASKRUNNING "Are you running?"
#define ASKSTATUS "What is your status?"
#define DONERUNNING "Finished running."
#define ASKFILE "Send me the file:"

/*implimentation details
  every sent message will be headed with a 1byte "info" section
  bit0:  valid
  bit1:  undeclared as of yet
  bit2:  EOF
  bit3:  check/bool statement
  bit4:  exec file
  bit5:  data file
  bit6:  status
  bit7:  error


*/

void manageTask(node* head);
int getMessageType(char* header);

void handleTaskOne(node* task);
void handleTaskTwo(node* task);
void handleTaskThree(node* task);
void handleTaskFour(node* task);
void handleTaskFive(node* task);
void handleTaskSix(node* task);
int readSocketIntoBuf(int socket, void* buf, int bufSize);

#endif

