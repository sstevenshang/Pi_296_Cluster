#include "taskManager.h"

int messageBufSize = 4096;
char messageBuf[4096];

ssize_t readSocketIntoBuf(int socket, void* buf, int bufSize){
  ssize_t bytesRead = read(socket, buf, bufSize);
  if(bytesRead == -1){
    if(errno == EWOULDBLOCK){ return 0;}
    fprintf(stderr, "Invalid read from socket %d\n", socket);
    return -1;
  } else {
    return bytesRead;
  }
}

ssize_t writeBufIntoSocket(int socket, void* buf, int bufSize){
  ssize_t bytesWrote = write(socket, buf, bufSize);
  if(bytesWrote == -1){
    if(errno == EWOULDBLOCK){ return 0;}
    fprintf(stderr, "Invalid write to socket %d\n", socket);
    return -1;
  } else {
    return bytesWrote;
  }
}


void manageTaskMaster(node* head){
  if(head == NULL){return;}

  node* tmp = head;
  while(tmp != NULL){
    puts("hitTask");
    int checkVal;
    if(tmp->taskNo == 0){ //no task assigned
puts("notask");      //cheating here, fix the scheduler
tmp->taskNo = 3;
tmp->taskPos = 0;
task* tmpTask = malloc(sizeof(task));
tmp->tmpTask = tmpTask;
tmpTask->file_name = "./testFiles/test1.txt";
    } else if(tmp->taskNo == 1){
      checkVal = handleTaskOne(tmp);
      if(checkVal == 0){puts("WORKERD!!!!!!");}
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

void addInfo(void* info, void* buf, size_t bufSize){ //assumes buf has space
/*  void* tmp = (void*)malloc(bufSize);
  //memset(buf, '\0', bufSize);
  if((*(int*)info) != ((int)iData) && (*(int*)info) != ((int)iExe)){
  memmove(info, buf, 1);
  memmove(buf, tmp+1, bufSize-1);
  } else {
    puts("FIXME"); return;
  }
  free(buf);
  buf = tmp;
*/
}

void resetHelper(node* task){
  task->taskPos ++;
  task->bufPos = 0;
  task->bufWIP = 0;
}

int updateBuf(node* task, ssize_t bytes){
  if(bytes == -1){ return -1;}
  task->bufPos += bytes;
  return 1;
}

void taskDone(node* task){
  task->bufPos = 0;
  task->taskPos = 0;
  task->taskNo = 5;
}

void initateTaskBuf(node* task, char* info, char* message){
    void* myBuf = task->buf;
    memset(myBuf, '\0', task->bufSize);
    strcpy(myBuf, info);
    strcat(myBuf, message);
}

int handleTaskOne(node* task){
  
    
    int pos = task->taskPos;
    switch(pos){
      case 0:{
	puts("case0");
	ssize_t bytesRead = readSocketIntoBuf(task->socket_fd, task->buf + task->bufPos, task->bufSize - task->bufPos);
fprintf(stdout, "bytes read : %zu, cur buf %s\n", bytesRead, task->buf);	if(bytesRead != (ssize_t)(strlen(RECEIVEME) +2 - task->bufPos)){
	  return updateBuf(task, bytesRead);
	} else {
	  if (strcmp(RECEIVEME, (task->buf+1)) != 0){
	    fprintf(stderr, "didn't receive correct value, reseting node");
	    return -1;
 	  } else {
	    resetHelper(task);
	  }
	}
	
      }case 1:{
	puts("case1");
	//strcpy(task->buf, RECEIVED);

	initateTaskBuf(task, iBool, RECEIVED);
    fprintf(stdout, "%s is buf\n", task->buf);
	ssize_t bytesWrote = writeBufIntoSocket(task->socket_fd, task->buf + task->bufPos, strlen(RECEIVED) + 2 - task->bufPos);

	if(bytesWrote != (ssize_t)(strlen(RECEIVED) +2- task->bufPos)){
	  return updateBuf(task, bytesWrote);
	} else {
	  resetHelper(task);
	}
	
      }case 2:{
	puts("case2");
	ssize_t bytesRead = readSocketIntoBuf(task->socket_fd, task->buf + task->bufPos, task->bufSize - task->bufPos);
        if(bytesRead != (ssize_t)(strlen(READY) +2 - task->bufPos)){
          return updateBuf(task, bytesRead);
        } else {
          if (strcmp(READY, (task->buf+1)) != 0){
            fprintf(stderr, "didn't receive correct value, reseting node");
            return -1;
          } else {
            resetHelper(task);
          }
        }
	taskDone(task);
	
      }
    }  
  //expects a message RECEIVEME
  
  //sends RECEIVED
  //recieves READY
  return 0;
}

int handleTaskTwo(node* task){
  return 0;
}
int handleTaskThree(node* task){
  //info is 1 byte, size is 4 bytes, name is null terminated

  //case 0, send, case1: read
  int pos = task->taskPos;
    switch(pos){
      case 0:{
        puts("case0");
	if(task->bufWIP == 0){
	  //task->bufWIP = 1;
        initateTaskBuf(task, iData, task->tmpTask->file_name);
    fprintf(stdout, "%s is buf\n", task->buf);
	  task->tmpTask->file_stream = fopen(task->tmpTask->file_name, "rb");
	  if(task->tmpTask->file_stream == NULL){ perror("bad file name given for task 3\n"); return -1;}
	  task->bufWIP = 1;
	} else { 
	char* tmp = "";
	 initateTaskBuf(task, iData, tmp);
	}
	puts("writing");
	ssize_t startPos = strlen(task->buf) +1;
	

	int len = read(fileno(task->tmpTask->file_stream), task->buf + startPos, task->bufSize - startPos);
	fprintf(stdout, "len is %d\n", len);
	if(len == -1){
		fprintf(stderr, "error reading fd in task three"); return -1;
	} else if( len == 0){
	  resetHelper(task);
	} else {
	fprintf(stdout, "writing %s\n", task->buf+1);
	  ssize_t bytesWrote = writeBufIntoSocket(task->socket_fd, task->buf+1, len + startPos-1);
	  return updateBuf(task, bytesWrote);
	}

	}case 1:{
        puts("case1");
	
        ssize_t bytesRead = readSocketIntoBuf(task->socket_fd, task->buf + task->bufPos, task->bufSize - task->bufPos);
        if(bytesRead != (ssize_t)(strlen(READY) +2 - task->bufPos)){
          return updateBuf(task, bytesRead);
        } else {
          if (strcmp(READY, (task->buf+1)) != 0){
            fprintf(stderr, "didn't receive correct value, reseting node");
            return -1;
          } else {
            resetHelper(task);
          }
        }
        taskDone(task);

      }
    }
  //expects a message RECEIVEME

  //sends RECEIVED
  //recieves READY
  return 0;
}


int handleTaskFour(node* task){
  return 0;
}
int handleTaskFive(node* task){
  return 0;
}
int handleTaskSix(node* task){
  return 0;
}

/*
 *Going to use a s
 *
 *
 *
 */
void manageTaskWorker(node* task){
    if(task == NULL){ fprintf(stderr, "worker has no head, ERROR\n"); return ;}
    int checkVal;
    if(task->taskNo == 0){ //startup
      checkVal =handleTaskThreeWorker(task); 
      if(checkVal == 0){puts("WORKED");}
      int runExec = 1;
      if(runExec == 1 && checkVal == 0){ runBinaryFileT(task->fileName);}
      //task->taskNo = 4;
    } else if(task->taskNo == 1){  //get file
      handleTaskOneWorker(task);
    } else if(task->taskNo == 2){  //send file
      handleTaskTwoWorker(task);
    } else if(task->taskNo == 3){  //send exe
      handleTaskThreeWorker(task);
    } else if(task->taskNo == 4){  //wait cycles
      handleTaskFourWorker(task);
    } else {
      fprintf(stderr, "worker has weird TaskNo, investigate\n");
    }
}

int handleTaskZeroWorker(node* task){//startup
  int pos = task->taskPos;
  switch(pos){

    case 0: {
        puts("case0");
        initateTaskBuf(task, iBool, RECEIVEME);
        ssize_t bytesWrote = writeBufIntoSocket(task->socket_fd, task->buf + task->bufPos, strlen(RECEIVEME) + 2 - task->bufPos);
        fprintf(stdout, "wrote %zu bytes\n", bytesWrote);if(bytesWrote != (ssize_t)(strlen(RECEIVEME) + 2 - task->bufPos)){
          return updateBuf(task, bytesWrote);
        } else {
          resetHelper(task);
        }


    } case 1: {
puts("case1");
	
        ssize_t bytesRead = readSocketIntoBuf(task->socket_fd, task->buf + task->bufPos, task->bufSize - task->bufPos);
        if(bytesRead != (ssize_t)(strlen(RECEIVED) +2 - task->bufPos)){
          return updateBuf(task, bytesRead);
        } else {
          if (strcmp(RECEIVED, (task->buf+1)) != 0){
            fprintf(stderr, "didn't receive correct value, reseting node");
            return -1;
          } else {
            resetHelper(task);
          }
        }

    } case 2: {
puts("case2");
       
        initateTaskBuf(task, iStatus, READY);
        ssize_t bytesWrote = writeBufIntoSocket(task->socket_fd, task->buf + task->bufPos, strlen(READY)+2 - task->bufPos);
        if(bytesWrote != (ssize_t)(strlen(READY) + 2 - task->bufPos)){
          return updateBuf(task, bytesWrote);
        } else {
          resetHelper(task);
        }
        task->taskNo = 4;
        return 0;
    }
  }



  return 0;
}

int handleTaskOneWorker(node* task){
  return 0;
}
int handleTaskTwoWorker(node* task){
  return 0;
}
int handleTaskThreeWorker(node* task){
  int pos = task->taskPos;
  switch(pos){

    case 0: {
        puts("case0");
	ssize_t bytesRead = 0;
        if(task->bufWIP == 0){
	  	bytesRead = readSocketIntoBuf(task->socket_fd, task->buf + task->bufPos, task->bufSize - task->bufPos);
		size_t nameLen = strlen(task->buf+1);
		if(nameLen <= 1){ return -1;}
		char temp[1024];
		task->fileName = strcpy(temp, task->buf);
		fprintf(stdout, "gotten filename is %s\n", task->fileName);
		//task->tmpFS = fopen(temp, "wb+");
		task->tmpFP = open(task->fileName, O_CREAT | O_RDWR, S_IRWXG | S_IRWXU);
		//if(task->tmpFS == NULL){ perror("tmpFS NULL"); return -1;}
		task->bufPos = bytesRead;
		task->taskPos = 1;
		task->bufWIP = 1;
	
        write(task->tmpFP, task->buf + nameLen + 2, bytesRead-2 - nameLen);
	}	
    } case 1: {
puts("case1");
        task->bufPos = 0;
        ssize_t bytesRead = readSocketIntoBuf(task->socket_fd, task->buf + task->bufPos, task->bufSize - task->bufPos);
        if(bytesRead != 0){
		puts("reading more data"); puts(task->buf);
		write(task->tmpFP, task->buf +1, bytesRead - 1);
		return 1;
	}
	resetHelper(task);

    } case 2: {
puts("case2");
	initateTaskBuf(task, iStatus, READY);
	ssize_t bytesWrote = writeBufIntoSocket(task->socket_fd, task->buf + task->bufPos, strlen(READY) + 2 - task->bufPos);
	if(bytesWrote != (ssize_t)(strlen(READY)+2-task->bufPos)){
		return updateBuf(task, bytesWrote);
	} else {
		resetHelper(task);
	}
	task->taskNo = 4;
        return 0;
    }
  }




  return 0;
}
int handleTaskFourWorker(node* task){
  return 0;
}


void runBinaryFileT(char* name){
    if(name == NULL){fprintf(stderr, "trying ot exec a null\n"); exit(0);}
    pthread_t myThread;
    pthread_attr_t* myAttr = NULL;
    pthread_create(&myThread, myAttr, &threadManagerT, (void*)name);
    pthread_join(myThread, NULL);
}

void* threadManagerT(void* arg){
    execlp((char*)arg, (char*)arg, NULL);
    fprintf(stderr, "exec returned");
    return (void*)-1;
}
