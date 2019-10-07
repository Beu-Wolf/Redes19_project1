#include "clientcommands.h"

/*
 * Validate command and communicate with server
 */
void processRegister(int fdUDP, char** parsedInput,
    addressInfoSet newAddrInfoSet) {

    int len = arglen(parsedInput);
    if(len != 2) {
      fprintf(stderr, INVALID_RG_ARGS);
      return;
    }

    if(userID != 0) {
        printf("This session is already registered\n");
        return;
    }
    sendRegister(fdUDP, parsedInput, newAddrInfoSet);
    receiveRegister(fdUDP, parsedInput);
}

void sendRegister(int fdUDP, char** parsedInput, addressInfoSet newAddrInfoSet) {
    char sendMsg[BUFFER_SIZE];


    memset(sendMsg, 0, BUFFER_SIZE);
    sprintf(sendMsg, "REG %s\n", parsedInput[1]);
    printf("sending %ld bytes: |%s|\n", strlen(sendMsg), sendMsg);

    if(sendto(fdUDP, sendMsg, strlen(sendMsg) , 0, newAddrInfoSet.res_UDP->ai_addr,
            newAddrInfoSet.res_UDP->ai_addrlen) == -1) fatal(strerror(errno));
}

void receiveRegister(int fdUDP, char** parsedInput) {
    char receivedMessage[BUFFER_SIZE];
    char** args;

    memset(receivedMessage, 0, BUFFER_SIZE);
    if(recvfrom(fdUDP, receivedMessage, BUFFER_SIZE, 0, NULL, NULL) == -1) fatal(UDPRECV_ERROR);

    args = tokenize(receivedMessage);

    stripnewLine(args[1]);
    if(!strcmp(args[1], "OK")) {
        userID = strtol(parsedInput[1], NULL, 0);
        printf("userID registered (id: %d)\n", userID);
    } else if (!strcmp(args[1], "NOK")){
        printf("error: invalid userID\n");
    }
}
