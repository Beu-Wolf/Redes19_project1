#include "clientcommands.h"


void processTopicPropose(char** parsedInput) {
    printf("Want to propose topic\n");
}

void sendTopicPropose(int fdUDP, char** parsedInput, addressInfoSet newAddrInfoSet) {
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

    stripnewLine(tokenedMessage[0]);
    if(!strcmp(tokenedMessage[0], "ERR")) {
        printf("Error: Bad command\n");
    }

    stripnewLine(tokenedMessage[1]);
    if(!strcmp(tokenedMessage[1], "OK")) {
        printf("Topic successfuly created!\n");
    } else if(!strcmp(tokenedMessage[1], "DUP")) {
        printf("Topic already exists\n");
    } else if(!strcmp(tokenedMessage[1], "FUL")) {
        printf("Topic list full\n");
    } else if(!strcmp(tokenedMessage[1], "NOK")) {
        printf("Error while creating topic\n");
    }
}
