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
#include "clientcommands.h"


void readLineArgs(int argc, char* argv[], service* newService){
    int n, opt;
    n = gethostname(newService->serverIP, BUFFER_SIZE);
    if(n == -1) fatal(GETHOSTNAME_ERROR);

    strcpy(newService->port, DEFAULT_PORT);
    flags = GET_BY_NAME;

    while((opt = getopt(argc, argv, "n:p:")) != -1) {
        switch (opt) {
            case 'n':
                strcpy(newService->serverIP, optarg);
                printf("%s\n", newService->serverIP);
                flags = GET_BY_IP;
                break;
            case 'p':
                strcpy(newService->port, optarg);
                printf("%s\n", newService->port);
                break;
            default:
                break;
        }
    }
}

void setAddrStruct(service* newService, addressInfoSet* newAddrInfoSet){
    int n;

    memset(&(newAddrInfoSet->hints_TCP), 0, sizeof(newAddrInfoSet->hints_TCP));
    newAddrInfoSet->hints_TCP.ai_family = AF_INET;
    newAddrInfoSet->hints_TCP.ai_socktype = SOCK_STREAM;
    newAddrInfoSet->hints_TCP.ai_protocol = IPPROTO_TCP;

    if(flags == GET_BY_NAME){
        newAddrInfoSet->hints_TCP.ai_flags = AI_NUMERICSERV;
    } else if (flags == GET_BY_IP) {
        newAddrInfoSet->hints_TCP.ai_flags = AI_NUMERICHOST;
    }

    n = getaddrinfo(newService->serverIP, newService->port,
            &(newAddrInfoSet->hints_TCP), &(newAddrInfoSet->res_TCP));
    if(n != 0) {
        fatal(gai_strerror(n));
    }

    memset(&(newAddrInfoSet->hints_UDP), 0, sizeof(newAddrInfoSet->hints_UDP));
    newAddrInfoSet->hints_UDP.ai_family = AF_INET;
    newAddrInfoSet->hints_UDP.ai_socktype = SOCK_DGRAM;
    newAddrInfoSet->hints_UDP.ai_protocol = IPPROTO_UDP;


    if(flags == GET_BY_NAME){
        newAddrInfoSet->hints_UDP.ai_flags = AI_NUMERICSERV;
    } else if(flags == GET_BY_IP) {
        newAddrInfoSet->hints_UDP.ai_flags = AI_NUMERICHOST;
    }


    n = getaddrinfo(newService->serverIP, newService->port,
            &(newAddrInfoSet->hints_UDP), &(newAddrInfoSet->res_UDP));
    if(n != 0) {
        fatal(gai_strerror(n));
    }

}

char isRegistered() {
  return userID != -1;
}


int main(int argc, char* argv[]) {
    int inpSize;
    char* input, *cmd;
    char** parsedInput;

    service newService;
    addressInfoSet newAddrInfoSet;

    int fdUDP;

    char *topicList[100] = {0};
    char *questionList[100] = {0};

    readLineArgs(argc, argv, &newService);
    setAddrStruct(&newService, &newAddrInfoSet);


    fdUDP = socket(newAddrInfoSet.res_UDP->ai_family,
            newAddrInfoSet.res_UDP->ai_socktype, newAddrInfoSet.res_UDP->ai_protocol);

    if(fdUDP == -1) fatal(SOCK_CREATE_ERROR);

    userID = -1;
    selectedTopic = NULL;

    inpSize = INPUT_SIZE;
    input = (char*)malloc(inpSize * sizeof(char));
    if(input == NULL) fatal(ALLOC_ERROR);
    while(1) {
        readCommand(&input, &inpSize);
        parsedInput = tokenize(input);
        cmd = parsedInput[0];

        if(!cmd) {
            free(parsedInput);
            continue;
        }

        if(!strcmp(cmd, "reg") || !strcmp(cmd, "register"))
            processRegister(fdUDP, parsedInput, newAddrInfoSet);

        else if(!strcmp(cmd, "tl") || !strcmp(cmd, "topic_list"))
            processTopicList(fdUDP, parsedInput, newAddrInfoSet, topicList);

        else if(!strcmp(cmd, "ts") || !strcmp(cmd, "topic_select")) {
            processTopicSelect(parsedInput, topicList);

        } else if(!strcmp(cmd, "tp") || !strcmp(cmd, "topic_propose")) {
            processTopicPropose(fdUDP, parsedInput, newAddrInfoSet, topicList);

        } else if(!strcmp(cmd, "ql") || !strcmp(cmd, "question_list")) {
            processQuestionList(fdUDP, parsedInput, newAddrInfoSet, questionList);

        } else if(!strcmp(cmd, "qg") || !strcmp(cmd, "question_get")) {
            processQuestionGet(parsedInput, questionList, newAddrInfoSet);

        } else if(!strcmp(cmd, "qs") || !strcmp(cmd, "question_submit")) {
            processQuestionSubmit(parsedInput, newAddrInfoSet, questionList);

        } else if(!strcmp(cmd, "as") || !strcmp(cmd, "answer_submit")) {
            processAnswerSubmit(parsedInput, newAddrInfoSet);

        } else if(!strcmp(cmd, "exit")) {
            close(fdUDP);
            break;

        } else {
            printf("command not valid. Please try again\n");
        }
    
        free(parsedInput);
    }

    free(input);
    return 0;
}
