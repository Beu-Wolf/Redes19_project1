#include "clientcommands.h"

void processTopicList(char** parsedInput) {
    printf("Want topic list\n");
}

void sendTopicList(int fdUDP, addressInfoSet newAddrInfoSet) {
    int n;
    char* sendMsg = "LTP\n";

    n = sendto(fdUDP, sendMsg, strlen(sendMsg) , 0, newAddrInfoSet.res_UDP->ai_addr, 
            newAddrInfoSet.res_UDP->ai_addrlen);

    if(n == -1){
        printf("error message: %s\n", strerror(errno));
        exit(1);
    }

}

void receiveTopicList(int fdUDP, struct sockaddr_in receiveAddr, 
socklen_t receiveAddrlen, char** topicList ) {
    int n, i;
    char sendMsg[BUFFER_SIZE];
    char** tokenizedMessage;
    char* topicName;
    char* topicUserID;

    n = recvfrom(fdUDP, sendMsg, BUFFER_SIZE, 0, (struct sockaddr *) &receiveAddr,
     &receiveAddrlen);
    if(n == -1) exit(1);


    tokenizedMessage = tokenize(sendMsg);

    int topicNumber = strtol(tokenizedMessage[1], NULL, 0);
    
    if(topicNumber == 0) {
        printf("No topics to show\n");
    } else {
        for(i = 0; i < topicNumber; i++) {

            if(i == topicNumber - 1){
                stripnewLine(tokenizedMessage[i+2]);
            }

            topicName = strtok(tokenizedMessage[i+2], ":");
            topicList[i] = topicName;
            topicUserID = strtok(NULL, ":");


            printf("%d - %s (proposed by %s)\n", i, topicName, topicUserID);
        }
    }

}