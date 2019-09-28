#ifndef _UTIL_H_
#define _UTIL_H_

void fatal(char* buffer);
void readCommand(char** bufPtr, int* bufSize);
char **tokenize(char *string);
int arglen(char **args);

int sendString(int sockfd, char* buffer);
int recv_line(int sockfd, char** buffer, int* size);

void stripnewLine(char* str);

#endif