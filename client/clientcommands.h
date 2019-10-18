#ifndef __CLIENTCOMMANDS_H_
#define __CLIENTCOMMANDS_H_

#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <arpa/inet.h>
#include <errno.h>
#include <dirent.h>
#include "../lib/util.h"

#define INPUT_SIZE 1024

#define BUFFER_SIZE 2048
#define DEFAULT_PORT "58036"
#define CLIENT_TIMEOUT 10

extern char *optarg;

struct addrinfo* tcpInfo;
struct addrinfo* udpInfo;
char* userID;
char* selectedTopic;
char* selectedQuestion;

char isRegistered();

//Register
void processRegister(int fdUDP, char** parsedInput);
void sendRegister(int fdUDP, char** parsedInput);
void receiveRegister(int fdUDP, char** parsedInput);

//Topic List
void processTopicList(int fdUDP, char** parsedInput, char** topicList);
void sendTopicList(int fdUDP);
void receiveTopicList(int fdUDP, char** topicList );

//Topic Select
void processTopicSelect(char** parsedInput, char** topicList);

//Topic Propose
void processTopicPropose(int fdUDP, char** parsedInput, char** topicList);
void sendTopicPropose(int fdUDP, char** parsedInput);
void receiveTopicPropose(int fdUDP, char** topicList, char* topicName);

//Question List
void processQuestionList(int fdUDP, char** args, char **questionList);
void sendQuestionList(int fdUDP);
void receiveQuestionList(int fdUDP, char **questionList);

//Question Get
void processQuestionGet(char** parsedInput, char** questionList);
int sendQuestionGet();
void receiveQuestionGet(int fdTCP);
int writeQuestion(int fdTCP, char* userId, char* path);

//Question Submit
void processQuestionSubmit(char** parsedInput, char** questionList);
int sendQuestionSubmit(char** parsedInput);
void receiveQuestionSubmit(int fdUDP, char** parsedInput, char** questionList);

//Answer Submit
void processAnswerSubmit(char **args);
int sendAnswerSubmit(int fd, char *text, char *image);
int recvAnswerSubmit(int fd);

//Question Get
void processQuestionGet(char** parsedInput, char** questionList);
int sendQuestionGet();
void receiveQuestionGet(int fdTCP);

#endif
