#include "clientcommands.h"

/*
 * Validate command and communicate with server
 */
void processRegister(int fdUDP, char** parsedInput) {
    if(isRegistered()) {
        printf("This session is already registered\n");
        return;
    }

    if(arglen(parsedInput) != 2) {
      fprintf(stderr, INVALID_RG_ARGS);
      return;
    }

    sendRegister(fdUDP, parsedInput);
    receiveRegister(fdUDP, parsedInput);
}

void sendRegister(int fdUDP, char** parsedInput) {
    char sendMsg[BUFFER_SIZE];

    memset(sendMsg, 0, BUFFER_SIZE);
    sprintf(sendMsg, "REG %s\n", parsedInput[1]);

    if(sendto(fdUDP, sendMsg, strlen(sendMsg) , 0, udpInfo->ai_addr, udpInfo->ai_addrlen) == -1)
        fatal(strerror(errno));
}

void receiveRegister(int fdUDP, char** parsedInput) {
    char receivedMessage[BUFFER_SIZE];
    char** args;

    memset(receivedMessage, 0, BUFFER_SIZE);
    if(recvfrom(fdUDP, receivedMessage, BUFFER_SIZE, 0, NULL, NULL) == -1)
        fatal(UDPRECV_ERROR);

    args = tokenize(receivedMessage);

    if(!strcmp(args[0], "ERR\n")) {
        printf("error: Something happened. Please try again\n");
        free(args);
        return;
    }

    stripnewLine(args[1]);
    if(!strcmp(args[1], "OK")) {
        userID = strdup(parsedInput[1]);
        printf("userID registered (id: %s)\n", userID);
    } else if (!strcmp(args[1], "NOK")){
        printf("error: invalid userID\n");
    }

    free(args);
}
