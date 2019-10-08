#ifndef _UTIL_H_
#define _UTIL_H_

#include <stdio.h>

// Error messages
#define ALLOC_ERROR "Allocation Error"
#define FILEREAD_ERROR "Reading from file"
#define FILEOPEN_ERROR "Opening File"
#define DIROPEN_ERROR "Opening Dir"
#define DIRCLOSE_ERROR "Closing Dir"
#define SOCK_CREATE_ERROR "Creating Socket"
#define SOCK_BIND_ERROR "Binding Socket"
#define SOCK_CONN_ERROR "Connecting Socket"
#define SOCK_ACPT_ERROR "Accepting Socket"
#define UDPRECV_ERROR "Receiving UDP"
#define GETHOSTNAME_ERROR "Getting host name"
#define GETADDRINFO_ERROR "Getting address info"
#define FORK_ERROR "in fork"
#define SELECT_ERROR "in select"
#define WAIT_ERROR "in wait child"
#define SERVER_ERR "Error in server, please try again"

#define NOT_REGISTERED_ERROR "You must be registered before performing any action\n"
#define NO_TOPIC_SELECTED_ERROR "You must have a topic selected\n"
#define QSIZE_ERROR "Question size exceeded\n"
#define FILE_NOT_AVAILABLE_ERROR "Given file is not available\n"
#define TOPIC_ERROR "Topic names must be alphanumeric and have no more than 10 characters\n"
#define INVALID_RG_ARGS "Invalid arguments.\nUsage: register/reg <userID>\n"
#define INVALID_TL_ARGS "Invalid arguments.\nUsage: topic_list/tl\n"
#define INVALID_TS_ARGS "Invalid arguments.\nUsage: topic_select <topic>/ts <topic_number>\n"
#define INVALID_TP_ARGS "Invalid arguments.\nUsage: topic_propose/tp <topic>\n"
#define INVALID_QL_ARGS "Invalid arguments.\nUsage: question_list/ql\n"
#define INVALID_QS_ARGS "Invalid arguments.\nUsage: question_submit/qs <question> <text_file> [<image_file.ext>]\n"
#define INVALID_QS_IMGEXT "Invalid image extension. Please select file with a permitted file extension\n"

// Protocol consts
#define TOPIC_MAXLEN 10
#define QUESTION_MAXLEN 10
#define MAXTOPICS 99


void printArgs(char** buffer);
void fatal(const char* buffer);
void readCommand(char** bufPtr, int* bufSize);
char **tokenize(char *string);
int arglen(char **args);
char *safestrcat(char *dest, char *src);

char isPositiveNumber(char* str);

int sendTCPstring(int sockfd, char* buffer);
int recvTCPline(int sockfd, char** buffer, int* size);
int recvTCPword(int sockfd, char** buffer, int* size);

int sendTCPfile(int sockfd, FILE* filefd);
int recvTCPfile(int sockfd, unsigned long long fileSize, FILE* filefd);

void stripnewLine(char* str);

int validate(char* topicName, int len);
int isValidTopic(char* topicName);

#endif
