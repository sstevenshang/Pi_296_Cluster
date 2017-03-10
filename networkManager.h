#ifndef _NETWORKMANAGER_H_
#define _NETWORKMANAGER_H_


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

  at any one time, exactly two bits, one being bit0, must be present for any message to go through and be processed
  if exactly no bits are presents, it "should" be a basic ping.
  
*/

//code only code? (useable by both)
int getMessageType(char* header);


//client only code
int getFdForWriteFile(char* name);
int setUpClient(char* addr, char* port);
int cleanUpClient(int socket);
char* getBinaryFile(int socket, char* name);
void runBinaryFile(char* name);
void* threadManager(void* arg);
void resetPipeClient(int socket);
//server only code
int addAnyIncomingConnections();
void slaveManager();
int getFdForReadFile(char* name);
int setUpServer(char* addr, char* port);
int cleanUpServer(int socket);
int sendBinaryFile(int socket, char* name);
void resetPipeServer(int socket);
#endif
