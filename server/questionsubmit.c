#include "commands.h"


void processQuestionSubmit(int fdTCP) {
    char* userId, *topic;
    int size;

    size = 6; // size of userID plus one
    userId = (char*) malloc(sizeof(char) * size);
    if(!userId) fatal(ALLOC_ERROR);

    recvTCPword(fdTCP, &userId, &size);
    printf("UserID: |%s|\n", userId);

    // Read topic
    size = 11; // size of userID plus one
    topic = (char*) malloc(sizeof(char) * size);
    if(!topic) fatal(ALLOC_ERROR);
    memset(topic, 0, 11);

    recvTCPword(fdTCP, &topic, &size);
    printf("Topic: |%s|\n", topic);
    

    //9 = max size of protocol mesage for question submit
    // questionSubmitStatus = (char*) malloc(sizeof(char)*9);      
    // if(!questionSubmitStatus) fatal(ALLOC_ERROR);
    // memset(questionSubmitStatus, 0, 9);

    free(userId);
}