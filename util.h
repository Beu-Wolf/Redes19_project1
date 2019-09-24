#ifndef _UTIL_H_
#define _UTIL_H_

void fatal(char* buffer);
void readCommand(char** bufPtr, int* bufSize);
char **tokenize(char *string);

int sendString(int sockfd, char* buffer);
int recv_line(int sockfd, char** buffer, int* size);

#endif
