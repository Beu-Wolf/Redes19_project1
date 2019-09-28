#ifndef _UTIL_H_
#define _UTIL_H_

// Error messages
#define NOT_REGISTERED_ERROR "You must be registered before performing any action\n"
#define INVALID_RG_ARGS "Invalid arguments.\nUsage: register/reg <userID>\n"
#define INVALID_TL_ARGS "Invalid arguments.\nUsage: topic_list/tl\n"
#define INVALID_TS_ARGS "Invalid arguments.\nUsage: topic_select <topic>/ts <topic_number>\n"

void fatal(char* buffer);
void readCommand(char** bufPtr, int* bufSize);
char **tokenize(char *string);
int arglen(char **args);

int sendString(int sockfd, char* buffer);
int recv_line(int sockfd, char** buffer, int* size);

void stripnewLine(char* str);

#endif
