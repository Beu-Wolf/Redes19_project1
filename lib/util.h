#ifndef _UTIL_H_
#define _UTIL_H_

#include <stdio.h>

// Error messages
#define ALLOC_ERROR "Allocation Error"
#define FILEREAD_ERROR "Reading from file"
#define FILEOPEN_ERROR "Opening File"
#define DIROPEN_ERROR "Opening Dir"
#define DIRCLOSE_ERROR "Closing Dir"
#define DIRCREATE_ERROR "Creating Dir"
#define SOCK_CREATE_ERROR "Creating Socket"
#define SOCK_BIND_ERROR "Binding Socket"
#define SOCK_CONN_ERROR "Connecting Socket"
#define SOCK_ACPT_ERROR "Accepting Socket"
#define UDPRECV_ERROR "Receiving UDP"
#define GETHOSTNAME_ERROR "Getting host name"
#define GETADDRINFO_ERROR "Getting address info"
#define FORK_ERROR "in fork"
#define SELECT_ERROR "in select"
#define STRDUP_ERROR "in strdup"
#define WAIT_ERROR "in wait child"
#define SERVER_ERROR "Error in server, please try again"
#define RECV_TCP_ERROR "Error reading from TCP socket"
#define FPUTS_ERROR "Error Writing in file"

#define NOT_REGISTERED_ERROR "You must be registered before performing any action\n"
#define NO_TOPIC_SELECTED_ERROR "You must have a topic selected\n"
#define NO_QUESTION_LOADED_ERROR "See available questions with command question_get (qg)\n"
#define QSIZE_ERROR "Question size exceeded\n"
#define FILE_NOT_AVAILABLE_ERROR "Cannot access file: No such file\n"
#define IMAGE_NOT_AVAILABLE_ERROR "Cannot access image: No such file\n"
#define NO_QUESTION_LIST "No question list store. Please run question_list/ql\n"
#define TOPIC_ERROR "Topic names must be alphanumeric and have no more than 10 characters\n"
#define QUESTION_ERROR "Question names must be alphanumeric and have no more than 10 characters\n"
#define INVALID_RG_ARGS "Invalid arguments.\nUsage: register/reg <userID>\n"
#define INVALID_TL_ARGS "Invalid arguments.\nUsage: topic_list/tl\n"
#define INVALID_TS_ARGS "Invalid arguments.\nUsage: topic_select <topic>/ts <topic_number>\n"
#define INVALID_TP_ARGS "Invalid arguments.\nUsage: topic_propose/tp <topic>\n"
#define INVALID_QL_ARGS "Invalid arguments.\nUsage: question_list/ql\n"
#define INVALID_QS_ARGS "Invalid arguments.\nUsage: question_submit/qs <question> <text_file> [<image_file.ext>]\n"
#define INVALID_AS_ARGS "Invalid arguments.\nUsage: answer_submit/as <text_file> [image_file.ext]\n"
#define INVALID_QS_IMGEXT "Invalid image extension. Please select file with a permitted file extension\n"
#define INVALID_QG_ARGS "Invalid arguments.\nUsage: question_get <question>/qg <question_number>\n"

#define MAX(A,B) ((A)>= (B) ? (A):(B))
#define MIN(A,B) ((A)<= (B) ? (A):(B))

// Protocol consts
#define TOPIC_MAXLEN 10
#define QUESTION_MAXLEN 10
#define MAXTOPICS 99
#define MAXQUESTIONS 99
#define MAXANSWERS 99
#define MAX_PORT 0XFFFF

void printTopicList(char** topicList);

void printArgs(char** buffer);
void fatal(const char* buffer);
void readCommand(char** bufPtr, int* bufSize);
char **tokenize(char *string);
int arglen(char **args);
char *safestrcat(char *dest, char *src);
void resetPtrArray(char** array, int max);

char validPort(char* str);
char isPositiveNumber(char *str);
long toNonNegative(char *str);

int sendTCPstring(int sockfd, char* buffer, size_t n);
int recvTCPline(int sockfd, char** buffer, int* size);
int recvTCPword(int sockfd, char** buffer, int* size);
int recvTCPchar(int sockfd, char* p);

int sendTCPfile(int sockfd, FILE* filefd);
int recvTCPfile(int sockfd, unsigned long long fileSize, FILE* filefd);

void stripnewLine(char* str);

int validate(char* topicName, int len);
int isValidTopic(char* topicName);
int isValidQuestion(char* questionName);
long fileSize(FILE *file);

void clearSocket(int fdTCP);

void setSocketTimeout(int fd, int seconds);
void timedOut();

#endif
