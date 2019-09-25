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
#include "util.h"

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


void processRegister(char** parsedInput) {
    printf("Want to register\n");
}

void sendRegister(int fdUDP, char** parsedInput, addressInfoSet newAddrInfoSet) {
    int n;
    char sendMsg[BUFFER_SIZE];
    memset(sendMsg, 0, BUFFER_SIZE);

    sprintf(sendMsg, "REG %s\n", parsedInput[1]);

    printf("%s:%ld\n", sendMsg, strlen(sendMsg));

    n = sendto(fdUDP, sendMsg, strlen(sendMsg) , 0, newAddrInfoSet.res_UDP->ai_addr, 
            newAddrInfoSet.res_UDP->ai_addrlen);

   
    if(n == -1){
        printf("error message: %s\n", strerror(errno));
        exit(1);
    }

}

void receiveRegister(int fdUDP, char** parsedInput,
struct sockaddr_in receiveAddr, socklen_t receiveAddrlen, int* userID) {
    int n;

    char receivedMessage[BUFFER_SIZE];
    char ** tokenedMessage;

    memset(receivedMessage, 0, BUFFER_SIZE);


    n = recvfrom(fdUDP, receivedMessage, BUFFER_SIZE, 0, (struct sockaddr *) &receiveAddr, 
    &receiveAddrlen);

    if(n == -1) exit(1);

    tokenedMessage = tokenize(receivedMessage);
    if(!strcmp(tokenedMessage[1], "OK\n") && *userID == 0) {
        *userID = strtol(parsedInput[1], NULL, 0);
        printf("userID: %d\n", *userID);
        
        printf("userID registered\n");
    } else if (!strcmp(tokenedMessage[1], "NOK\n")){
        printf("error: invalid userID\n");
        
    } else if(*userID != 0){
        printf("Cant register more than once\n");
    }
    

}

void processTopicList(char** parsedInput) {
    printf("Want topic list\n");
}

void processTopicSelect(char** parsedInput, short argumentShort){
if(argumentShort) 
    printf("Want topic number\n");
else
    printf("Want topic\n");
}

void processTopicPropose(char** parsedInput) {
    printf("Want to propose topic\n");
}

void sendTopicPropose(int fdUDP, char** parsedInput, addressInfoSet newAddrInfoSet, 
int userID) {
    int n;
    char sendMsg[BUFFER_SIZE];

    sprintf(sendMsg, "PTP %d %s\n", userID, parsedInput[1]);

    printf("%s:%ld\n", sendMsg, strlen(sendMsg));

    n = sendto(fdUDP, sendMsg, strlen(sendMsg) , 0, newAddrInfoSet.res_UDP->ai_addr, 
            newAddrInfoSet.res_UDP->ai_addrlen);

   
    if(n == -1){
        printf("error message: %s\n", strerror(errno));
        exit(1);
    }

}

void receiveTopicPropose(int fdUDP, struct sockaddr_in receiveAddr, 
socklen_t receiveAddrlen ) {

    int n;

    char receivedMessage[BUFFER_SIZE];
    char ** tokenedMessage;

    n = recvfrom(fdUDP, receivedMessage, BUFFER_SIZE, 0, (struct sockaddr *) &receiveAddr, 
    &receiveAddrlen);

    if(n == -1) exit(1);

    tokenedMessage = tokenize(receivedMessage);

    if(!strcmp(tokenedMessage[0], "ERR\n")) {
        printf("Error: Bad command\n");
    }

    if(!strcmp(tokenedMessage[1], "OK\n")) {
        printf("Topic successfuly created!\n");
    } else if(!strcmp(tokenedMessage[1], "DUP\n")) {
        printf("Topic already exists\n");
    } else if(!strcmp(tokenedMessage[1], "FUL\n")) {
        printf("Topic list full\n");
    } else if(!strcmp(tokenedMessage[1], "NOK\n")) {
        printf("Error while creating topic\n");
    }
}

void processQuestionList(char** parsedInput) {
    printf("Want question list\n");
}

void processQuestionGet(char** parsedInput, short argumentShort) {
if(argumentShort) 
    printf("Want question number\n");
else
    printf("Want question\n");
}

void processQuestionSubmit(char** parsedInput) {
    printf("Want to submit question\n");
}

void processAnswerSubmit(char** parsedInput) {
    printf("Want to submit answer\n");
}


// TODO: Remove (debug function)
void printArgs(char** buffer) {
      int i = 0;
      while(buffer[i] != NULL) {
        printf("[%d] -> %s\n", i, buffer[i]);
        i++;
      }
}

int main(int argc, char* argv[]) {

    char hostname[BUFFER_SIZE];

    int inpSize;
    char* input;
    char** parsedInput;
    short argumentShort;

    service newService; 
    addressInfoSet newAddrInfoSet;

    struct sockaddr_in receiveAddr;
    socklen_t receiveAddrlen;

    int n;
    int fdTCP, fdUDP;

    ssize_t sentB, sendBLeft;

    char buffer[INET_ADDRSTRLEN];

    int userID = 0;

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
      printf("read: |%s|\n", input);
      parsedInput = tokenize(input);

      // printArgs(parsedInput);
      
      if(parsedInput[0] != NULL) {
        if(!strcmp(parsedInput[0], "register") || !strcmp(parsedInput[0], "reg")) {
          processRegister(parsedInput);
          sendRegister(fdUDP, parsedInput, newAddrInfoSet);
          receiveRegister(fdUDP, parsedInput, receiveAddr, receiveAddrlen,
          &userID);
        
        } else if(!strcmp(parsedInput[0], "topic_list") || 
        !strcmp(parsedInput[0], "tl")) {
          processTopicList(parsedInput);
        
        } else if(!strcmp(parsedInput[0], "topic_select")){
          argumentShort = 0;
          processTopicSelect(parsedInput, argumentShort);
        
        } else if(!strcmp(parsedInput[0], "ts")) {
          argumentShort = 1;
          processTopicSelect(parsedInput, argumentShort);
        
        } else if(!strcmp(parsedInput[0], "topic_propose") || 
        !strcmp(parsedInput[0], "tp")) {
            if(userID == 0) {
                printf("Can't send message without being registered\n");
            } else {
                processTopicPropose(parsedInput);
                sendTopicPropose(fdUDP, parsedInput, newAddrInfoSet, userID);
                receiveTopicPropose(fdUDP, receiveAddr, receiveAddrlen);
            }

        } else if(!strcmp(parsedInput[0], "question_list") || 
        !strcmp(parsedInput[0], "ql")) {
          processQuestionList(parsedInput);
        
        } else if(!strcmp(parsedInput[0], "question_get")){ 
          argumentShort = 0;
          processQuestionGet(parsedInput, argumentShort);
        
        } else if(!strcmp(parsedInput[0], "qg")) {
          argumentShort = 1;
          processQuestionGet(parsedInput, argumentShort);
        
        } else if(!strcmp(parsedInput[0], "question_submit") || 
        !strcmp(parsedInput[0], "qs")) {
          processQuestionSubmit(parsedInput);

        } else if(!strcmp(parsedInput[0], "answer_submit") || 
        !strcmp(parsedInput[0], "as")) {
        processAnswerSubmit(parsedInput); 
        
        } else if(!strcmp(parsedInput[0], "exit")) {
          free(parsedInput);
          exit(0);
        
        } else {
          printf("command not valid. Please try again\n");
        }
      }
      free(parsedInput);
    }

    free(input);

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
