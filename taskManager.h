#ifndef _TASK_MANAGER_H_
#define _TASK_MANAGER_H_

#include "node.h"
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#define READY "Ready for work."
#define RECEIVEME "Do you receive me?"
#define RECEIVED "Yes, I received you."
#define RUNNING "Running."
#define ASKRUNNING "Are you running?"
#define ASKSTATUS "What is your status?"
#define DONERUNNING "Finished running."
#define ASKFILE "Send me the file:"
#define iEOF "\x05"
#define iBool "\x09"
#define iExe "\x11"
#define iData "\x21"
#define iStatus "\x41"
#define iErr "\x81"

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


void manageTaskWorker(node* head);
void manageTaskMaster(node* head);
int getMessageType(char* header);
int updateBuf(node* task, ssize_t bytes);
void resetHelper(node* task); 
void taskDone(node* task);
void addInfo(void* info, void* buf, size_t bufSize);
int handleTaskOne(node* task);
int handleTaskTwo(node* task);
int handleTaskThree(node* task);
int handleTaskFour(node* task);
int handleTaskFive(node* task);
int handleTaskSix(node* task);
int handleTaskZeroWorker(node* task);
int handleTaskOneWorker(node* task);
int handleTaskTwoWorker(node* task);
int handleTaskThreeWorker(node* task);
int handleTaskFourWorker(node* task);

void runBinaryFileT(char* name);
void* threadManagerT(void* arg);

ssize_t readSocketIntoBuf(int socket, void* buf, int bufSize);
ssize_t writeBufIntoSocket(int socket, void* buf, int bufSize);
#endif

