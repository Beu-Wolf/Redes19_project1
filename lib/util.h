#ifndef _UTIL_H_
#define _UTIL_H_

// Error messages
#define NOT_REGISTERED_ERROR "You must be registered before performing any action\n"
#define INVALID_RG_ARGS "Invalid arguments.\nUsage: register/reg <userID>\n"
#define INVALID_TL_ARGS "Invalid arguments.\nUsage: topic_list/tl\n"
#define INVALID_TS_ARGS "Invalid arguments.\nUsage: topic_select <topic>/ts <topic_number>\n"
#define INVALID_TP_ARGS "Invalid arguments.\nUsage: topic_propose/tp <topic>\n"
#define INVALID_QS_ARGS "Invalid arguments.\nUsage: question_submit <question> <text_file> [<image_file.ext]\n"
#define INVALID_QS_IMGEXT "Invalid image extension. Please select file with a permitted file extension\n"


void fatal(char* buffer);
void readCommand(char** bufPtr, int* bufSize);
char **tokenize(char *string);
int arglen(char **args);
char *safestrcat(char *dest, char *src);

char isPositiveNumber(char* str);

int sendTCPstring(int sockfd, char* buffer);
int recvTCPline(int sockfd, char** buffer, int* size);

void stripnewLine(char* str);

#endif
