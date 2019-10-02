#include "clientcommands.h"

void processTopicList(int fdUDP, char** parsedInput,
    addressInfoSet newAddrInfoSet, char** topicList) {

    if(userID == 0) {
      fprintf(stderr, NOT_REGISTERED_ERROR);
      return;
    }

    if(arglen(parsedInput) != 1) {
      fprintf(stderr, INVALID_TL_ARGS);
      return;
    }

    sendTopicList(fdUDP, newAddrInfoSet);
    receiveTopicList(fdUDP, topicList);
}

void sendTopicList(int fdUDP, addressInfoSet newAddrInfoSet) {
    int n;
    char* sendMsg = "LTP\n";

    n = sendto(fdUDP, sendMsg, strlen(sendMsg) , 0, newAddrInfoSet.res_UDP->ai_addr,
            newAddrInfoSet.res_UDP->ai_addrlen);

    if(n == -1) fatal(strerror(errno));
}

void receiveTopicList(int fdUDP, char** topicList) {
    int n, i;
    char sendMsg[BUFFER_SIZE];
    char** args;
    char* topicName;
    char* topicUserID;

    n = recvfrom(fdUDP, sendMsg, BUFFER_SIZE, 0, NULL, NULL);
    if(n == -1) fatal(UDPRECV_ERROR);


    args = tokenize(sendMsg);

    long topicNumber = strtol(args[1], NULL, 10);

    if(topicNumber == 0) {
        printf("No topics to show\n");
    } else {
        for(i = 0; i < topicNumber; i++) {

            if(i == topicNumber - 1){
                stripnewLine(args[i+2]);
            }

            topicName = strtok(args[i+2], ":");
            topicList[i] = strdup(topicName);
            topicUserID = strtok(NULL, ":");


            printf("%d - %s (proposed by %s)\n", i, topicName, topicUserID);
        }
    }

}
