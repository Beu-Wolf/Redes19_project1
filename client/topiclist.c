#include "clientcommands.h"

void processTopicList(int fdUDP, char** parsedInput, char** topicList) {

    if(!isRegistered()) {
      fprintf(stderr, NOT_REGISTERED_ERROR);
      return;
    }

    if(arglen(parsedInput) != 1) {
      fprintf(stderr, INVALID_TL_ARGS);
      return;
    }

    sendTopicList(fdUDP);
    receiveTopicList(fdUDP, topicList);
}

void sendTopicList(int fdUDP) {
    char* sendMsg = "LTP\n";

    if(sendto(fdUDP, sendMsg, strlen(sendMsg) , 0, udpInfo->ai_addr, udpInfo->ai_addrlen) == -1)
        fatal(strerror(errno));
}

void receiveTopicList(int fdUDP, char** topicList) {
    int i;
    char* sendMsg;
    char** args;
    char* topicName;
    char* topicUserID;
    long topicNumber;

    if(!(sendMsg = (char*)malloc(sizeof(char) * BUFFER_SIZE))) fatal(ALLOC_ERROR);
    sendMsg = memset(sendMsg, 0, BUFFER_SIZE);

    if(recvfrom(fdUDP, sendMsg, BUFFER_SIZE, 0, NULL, NULL) == -1)
        fatal(UDPRECV_ERROR);

    args = tokenize(sendMsg);
    if(args[1] == NULL) {
        return;
    }
    topicNumber = strtol(args[1], NULL, 10);

    if(topicNumber == 0) {
        printf("No topics to show\n");
        return;
    }

    // backup old selectedTopic
    if(selectedTopic) selectedTopic = strdup(selectedTopic);
    resetPtrArray(topicList, MAXTOPICS+1);
    for(i = 0; i < topicNumber; i++) {
        if(i == topicNumber - 1)
            stripnewLine(args[i+2]);

        topicName = strtok(args[i+2], ":");
        topicList[i] = strdup(topicName);
        if(selectedTopic && !strcmp(selectedTopic, topicList[i])) {
            free(selectedTopic);
            selectedTopic = topicList[i];
        }
        topicUserID = strtok(NULL, ":");


        printf("%02d - %s (proposed by %s)\n", i+1, topicName, topicUserID);
    }
    free(sendMsg);
    free(args);
}

