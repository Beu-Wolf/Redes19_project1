#include "clientcommands.h"


void processTopicPropose(int fdUDP, char** parsedInput, addressInfoSet newAddrInfoSet){
    if(!isRegistered()) {
      fprintf(stderr, NOT_REGISTERED_ERROR);
      return;
    }
    if(arglen(parsedInput) != 2) {
      fprintf(stderr, INVALID_TP_ARGS);
      return;
    }

    if(!isValidTopic(parsedInput[1])) {
      fprintf(stderr, TOPIC_ERROR);
      return;
    }
    sendTopicPropose(fdUDP, parsedInput, newAddrInfoSet);
    receiveTopicPropose(fdUDP);
}

void sendTopicPropose(int fdUDP, char** parsedInput, addressInfoSet newAddrInfoSet) {
    char sendMsg[BUFFER_SIZE];

    sprintf(sendMsg, "PTP %d %s\n", userID, parsedInput[1]);

    printf("Sending %ld bytes: |%s|\n",  strlen(sendMsg), sendMsg);

    if(sendto(fdUDP, sendMsg, strlen(sendMsg) , 0, newAddrInfoSet.res_UDP->ai_addr,
            newAddrInfoSet.res_UDP->ai_addrlen) == -1) 
        fatal(strerror(errno));
}

void receiveTopicPropose(int fdUDP) {
    char receivedMessage[BUFFER_SIZE];
    char** args;

    if(recvfrom(fdUDP, receivedMessage, BUFFER_SIZE, 0, NULL, NULL) == -1)
        fatal(UDPRECV_ERROR);

    args = tokenize(receivedMessage);
    stripnewLine(args[0]);

    if(!strcmp(args[0], "ERR")) {
        printf("Error: Bad command\n");
        return;
    }

    if(arglen(args) < 2) return;

    stripnewLine(args[1]);
    if(!strcmp(args[1], "OK")) {
        printf("Topic successfuly created!\n");
    } else if(!strcmp(args[1], "DUP")) {
        printf("Topic already exists\n");
    } else if(!strcmp(args[1], "FUL")) {
        printf("Topic list full\n");
    } else if(!strcmp(args[1], "NOK")) {
        printf("Error while creating topic\n");
    }
}
