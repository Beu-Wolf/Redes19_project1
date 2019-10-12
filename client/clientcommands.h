#ifndef __CLIENTCOMMANDS_H_
#define __CLIENTCOMMANDS_H_

#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <arpa/inet.h>
#include <errno.h>
#include "../lib/util.h"

#define INPUT_SIZE 1024

#define BUFFER_SIZE 2048
#define DEFAULT_PORT "58036"


extern char *optarg;

enum flags {
    GET_BY_IP,
    GET_BY_NAME,
};

typedef struct service {
        char serverIP[BUFFER_SIZE];
        char port[BUFFER_SIZE];
} service;

typedef struct addressInfoSet {

    struct addrinfo hints_TCP;
    struct addrinfo* res_TCP;
    struct addrinfo hints_UDP;
    struct addrinfo* res_UDP;

} addressInfoSet;

enum flags flags;

int userID;

char* selectedTopic;
char* selectedQuestion;

char isRegistered();

//Register
void processRegister(int fdUDP, char** parsedInput,
    addressInfoSet newAddrInfoSet);
void sendRegister(int fdUDP, char** parsedInput,
    addressInfoSet newAddrInfoSet);
void receiveRegister(int fdUDP, char** parsedInput);

//Topic List
void processTopicList(int fdUDP, char** parsedInput,
    addressInfoSet newAddrInfoSet, char** topicList);
void sendTopicList(int fdUDP, addressInfoSet newAddrInfoSet);
void receiveTopicList(int fdUDP, char** topicList );

//Topic Select
void processTopicSelect(char** parsedInput, char** topicList);

//Topic Propose
void processTopicPropose(int fdUDP, char** parsedInput,
    addressInfoSet newAddrInfoSet);
void sendTopicPropose(int fdUDP, char** parsedInput, addressInfoSet newAddrInfoSet);
void receiveTopicPropose(int fdUDP);

//Question List
void processQuestionList(int fdUDP, char** args,
		addressInfoSet newAddrInfoSet, char **questionList);
void receiveQuestionList(int fdUDP, char **questionList);
void sendQuestionList(int fdUDP, addressInfoSet newAddrInfoSet);

//Question Submit
void processQuestionSubmit(char** parsedInput, addressInfoSet newAddrInfoSet);
int sendQuestionSubmit(char** parsedInput, addressInfoSet newAddrInfoSet);
void receiveQuestionSubmit(int fdUDP);

//Answer Submit
void processAnswerSubmit(char **args, addressInfoSet newAddrInfoSet);
int sendAnswerSubmit(int fd, char *text, char *image);
int recvAnswerSubmit(int fd);

//Question Get
void processQuestionGet(char** parsedInput, char** questionList, addressInfoSet newAddrInfoSet);
int sendQuestionGet(addressInfoSet newAddrInfoSet);

#endif
