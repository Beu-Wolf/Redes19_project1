#include "clientcommands.h"

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
struct sockaddr_in receiveAddr, socklen_t receiveAddrlen) {
    int n;

    char receivedMessage[BUFFER_SIZE];
    char ** tokenedMessage;

    memset(receivedMessage, 0, BUFFER_SIZE);


    n = recvfrom(fdUDP, receivedMessage, BUFFER_SIZE, 0, (struct sockaddr *) &receiveAddr, 
    &receiveAddrlen);

    if(n == -1) exit(1);

    tokenedMessage = tokenize(receivedMessage);

    stripnewLine(tokenedMessage[1]);
    if(userID == 0 && !strcmp(tokenedMessage[1], "OK")) {
        userID = strtol(parsedInput[1], NULL, 0);
        printf("userID registered (id: %d)\n", userID);
    } else if (!strcmp(tokenedMessage[1], "NOK")){
        printf("error: invalid userID\n");
        
    } else if(userID != 0){
        printf("Can't register more than once\n");
    }
    

}