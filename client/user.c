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


// TODO: Remove (debug function)
void printArgs(char** buffer) {
    int i = 0;
    while(buffer[i] != NULL) {
        printf("[%d] -> %s\n", i, buffer[i]);
        i++;
    }
}

void readLineArgs(int argc, char* argv[], service* newService){
    int n, opt;
    n = gethostname(newService->serverIP, BUFFER_SIZE);
    if(n == -1) exit(1);

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
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(n));
        exit(1);
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
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(n));
        exit(1);
    }

}

char isRegistered() {
  return userID != 0;
}


void processQuestionList(char** parsedInput) {
    printf("Want question list\n");
}

void processQuestionGet(char** parsedInput) {
    if(strlen(parsedInput[0]) == 2)
        printf("Want question number");
    else
        printf("Want question name");
}


void processAnswerSubmit(char** parsedInput) {
    printf("Want to submit answer\n");
}




int main(int argc, char* argv[]) {

    int inpSize;
    char* input, *cmd;
    char** parsedInput;

    service newService;
    addressInfoSet newAddrInfoSet;

    struct sockaddr_in receiveAddr;
    socklen_t receiveAddrlen;

    int n;
    int fdTCP, fdUDP;


    char buffer[INET_ADDRSTRLEN];

    char* topicList[50] = {0};

    readLineArgs(argc, argv, &newService);
    setAddrStruct(&newService, &newAddrInfoSet);


    fdUDP = socket(newAddrInfoSet.res_UDP->ai_family,
            newAddrInfoSet.res_UDP->ai_socktype, newAddrInfoSet.res_UDP->ai_protocol);

    if(fdUDP == -1) exit(1);

    inpSize = INPUT_SIZE;
    input = (char*)malloc(inpSize * sizeof(char));
    if(input == NULL) exit(1);
    while(1) {
        readCommand(&input, &inpSize);
        // printf("read: |%s|\n", input);
        parsedInput = tokenize(input);
        cmd = parsedInput[0];

        // printArgs(parsedInput);

        if(parsedInput[0] == NULL)
          continue;

        if(!strcmp(cmd, "reg") || !strcmp(cmd, "register"))
            processRegister(fdUDP, parsedInput, newAddrInfoSet, receiveAddr, receiveAddrlen);

        else if(!strcmp(cmd, "tl") || !strcmp(cmd, "topic_list"))
            processTopicList(fdUDP, parsedInput, newAddrInfoSet, receiveAddr, receiveAddrlen, topicList);

        else if(!strcmp(cmd, "ts") || !strcmp(cmd, "topic_select")) {
            processTopicSelect(parsedInput, topicList);

        } else if(!strcmp(cmd, "tp") || !strcmp(cmd, "topic_propose")) {
            processTopicPropose(fdUDP, parsedInput, newAddrInfoSet, receiveAddr, receiveAddrlen);

        } else if(!strcmp(cmd, "ql") || !strcmp(cmd, "question_list")) {
            processQuestionList(parsedInput);

        } else if(!strcmp(cmd, "qg") || !strcmp(cmd, "question_get")) {
            processQuestionGet(parsedInput);

        } else if(!strcmp(cmd, "qs") || !strcmp(cmd, "question_submit")) {
            processQuestionSubmit(parsedInput, newAddrInfoSet);

        } else if(!strcmp(cmd, "as") || !strcmp(cmd, "answer_submit")) {
            processAnswerSubmit(parsedInput);

        } else if(!strcmp(cmd, "exit")) {
          break;

        } else {
            printf("command not valid. Please try again\n");
        }
    
        free(parsedInput);
    }

    free(input);
    return 0;

    /*while(0) {
      fgets(sendMsg, BUFFER_SIZE, stdin);
      if(atoi(sendMsg) == 1) {


      n = sendto(fdUDP, "TestUDP\n", 8, 0, newAddrInfoSet.res_UDP->ai_addr,
      newAddrInfoSet.res_UDP->ai_addrlen);

      close(fdUDP);


      } else if(atoi(sendMsg) == 2){
      fdTCP = socket(newAddrInfoSet.res_TCP->ai_family,
      newAddrInfoSet.res_TCP->ai_socktype, newAddrInfoSet.res_TCP->ai_protocol);
      if(fdTCP == -1) exit(1);

      n = connect(fdTCP, newAddrInfoSet.res_TCP->ai_addr,
      newAddrInfoSet.res_TCP->ai_addrlen);
      if(n == -1) exit(1);

      strcpy(sendMsg, "Hello\n");
      sendBLeft = 7;
      while(sendBLeft > 0){
      sentB = write(fdTCP, sendMsg, sendBLeft);
      if(sentB <= 0) exit(1);

      sendBLeft -= sentB;

      close(fdTCP);
      }



      } else {
      printf("Error\n");
      break;
      }
      }*/

}
