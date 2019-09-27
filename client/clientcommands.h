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

#define BUFFER_SIZE 1024
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

//Register
void processRegister(char** parsedInput);
void sendRegister(int fdUDP, char** parsedInput, 
addressInfoSet newAddrInfoSet);
void receiveRegister(int fdUDP, char** parsedInput,
struct sockaddr_in receiveAddr, socklen_t receiveAddrlen);

//Topic List
void processTopicList(char** parsedInput);
void sendTopicList(int fdUDP, addressInfoSet newAddrInfoSet);
void receiveTopicList(int fdUDP, struct sockaddr_in receiveAddr, 
socklen_t receiveAddrlen, char** topicList );

//Topic Select
void processTopicSelect(char** parsedInput, short argumentShort, char** topicList);

//Topic Propose
void processTopicPropose(char** parsedInput);
void sendTopicPropose(int fdUDP, char** parsedInput, addressInfoSet newAddrInfoSet);
void receiveTopicPropose(int fdUDP, struct sockaddr_in receiveAddr, 
socklen_t receiveAddrlen );

#endif
