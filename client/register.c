#include "clientcommands.h"

void processRegister(char** parsedInput) {
    printf("Want to register\n");
}

void sendRegister(int fdUDP, char** parsedInput, addressInfoSet newAddrInfoSet) {
    int n;
    char sendMsg[BUFFER_SIZE];

    if(userID != 0) {
        printf("This session is already registered\n");
        return;
    }

    memset(sendMsg, 0, BUFFER_SIZE);
    sprintf(sendMsg, "REG %s\n", parsedInput[1]);
    printf("sending %ld bytes: |%s|\n", strlen(sendMsg), sendMsg);

    n = sendto(fdUDP, sendMsg, strlen(sendMsg) , 0, newAddrInfoSet.res_UDP->ai_addr,
            newAddrInfoSet.res_UDP->ai_addrlen);


    if(n == -1){
        printf("error message: %s\n", strerror(errno));
        exit(1);
    }

}

void receiveRegister(int fdUDP, char** parsedInput,
    struct sockaddr_in receiveAddr, socklen_t receiveAddrlen) {
    int n;

    char receivedMessage[BUFFER_SIZE];
    char ** args;

    memset(receivedMessage, 0, BUFFER_SIZE);


    n = recvfrom(fdUDP, receivedMessage, BUFFER_SIZE, 0, (struct sockaddr *) &receiveAddr,
            &receiveAddrlen);

    if(n == -1) exit(1);

    args = tokenize(receivedMessage);

    stripnewLine(args[1]);
    if(!strcmp(args[1], "OK")) {
        userID = strtol(parsedInput[1], NULL, 0);
        printf("userID registered (id: %d)\n", userID);
    } else if (!strcmp(args[1], "NOK")){
        printf("error: invalid userID\n");
    }


}
