#ifndef _NETWORKMANAGER_H_
#define _NETWORKMANAGER_H_

int getFdForWriteFile(char* name);
int setUpClient(char* addr, char* port);
int cleanUpClient(int socket);
char* getBinaryFile(int socket, char* name);
void runBinaryFile(char* name);
void* threadManager(void* arg);
void resetPipeClient(int socket);

int getFdForReadFile(char* name);
int setUpServer(char* addr, char* port);
int cleanUpServer(int socket);
int sendBinaryFile(int socket, char* name);
void resetPipeServer(int socket);
#endif
