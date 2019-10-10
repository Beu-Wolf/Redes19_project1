#include "clientcommands.h"

#define FINFO_BUFF_SIZE 50 

void processQuestionSubmit(char** parsedInput, addressInfoSet newAddrInfoSet) {
    int len = arglen(parsedInput);
    int fdTCP;

    // check if registered
    if(!isRegistered()) {
        printf(NOT_REGISTERED_ERROR);
        return;
    }

    // check selected topic 
    if(selectedTopic == NULL) {
        printf(NO_TOPIC_SELECTED_ERROR);
        return;
    } 

    // check #arguments
    if(len < 3 || len > 4){
        printf(INVALID_QS_ARGS);
        return;
    }

    // check question length
    if(strlen(parsedInput[1]) > QUESTION_MAXLEN) {
        printf(QSIZE_ERROR);
        return;
    }

    // check if readable files
    if(access(parsedInput[2], R_OK) || (len == 4 && access(parsedInput[3], R_OK))) {
        printf(FILE_NOT_AVAILABLE_ERROR);
        return;
    }


    fdTCP = sendQuestionSubmit(parsedInput, newAddrInfoSet);

    if(fdTCP == -1)
        return;

    receiveQuestionSubmit(fdTCP);
    close(fdTCP);
}

int sendQuestionSubmit(char** parsedInput, addressInfoSet newAddrInfoSet) {
    long questionFileSz, imgSz;
    int fdTCP, n;
    char* buffer;
    char* imgExt;
    FILE* imgFd;
    FILE* questionFD; 
    char hasImg;

    // prep data
    questionFD =  fopen(parsedInput[2], "r");
    if(questionFD == NULL)
        fatal(FILEREAD_ERROR);

    // get file size
    fseek(questionFD, 0, SEEK_END);
    questionFileSz = ftell(questionFD);
    fseek(questionFD, 0L, SEEK_SET);

    if ((hasImg = arglen(parsedInput) == 4)) {
        imgFd = fopen(parsedInput[3], "r");
        if(imgFd == NULL)
            fatal(FILEREAD_ERROR);

        // get img extension
        strtok(parsedInput[3], ".");
        imgExt = strtok(NULL, ".");
        printf("img extension: %s\n", imgExt);
        if(imgExt == NULL || strlen(imgExt) != 3) {
            printf(INVALID_QS_IMGEXT);
            return -1;
        }

        // img size
        fseek(imgFd, 0, SEEK_END);
        imgSz = ftell(imgFd);
        fseek(imgFd, 0L, SEEK_SET);
    }

    fdTCP = socket(newAddrInfoSet.res_TCP->ai_family, newAddrInfoSet.res_TCP->ai_socktype,
    newAddrInfoSet.res_TCP->ai_protocol);

    if(fdTCP == -1) fatal(SOCK_CREATE_ERROR);

    n = connect(fdTCP, newAddrInfoSet.res_TCP->ai_addr, newAddrInfoSet.res_TCP->ai_addrlen);
    if(n == -1) fatal(SOCK_CONN_ERROR);


    // send request
      // 1: send until file data
    buffer = (char*) malloc(sizeof(char)*FINFO_BUFF_SIZE);
    if(!buffer) fatal(ALLOC_ERROR);
    memset(buffer, 0, FINFO_BUFF_SIZE);

    sprintf(buffer, "QUS %d %s %s %ld ", userID, selectedTopic, 
        parsedInput[1], questionFileSz);
    sendTCPstring(fdTCP, buffer, strlen(buffer));

      // 2: send file data
    sendTCPfile(fdTCP, questionFD);

      // 3
    if(hasImg) {
        memset(buffer, 0, FINFO_BUFF_SIZE);
        sprintf(buffer, "%d %s %ld ", 1, imgExt, imgSz);
        sendTCPstring(fdTCP, buffer, strlen(buffer));

        sendTCPfile(fdTCP, imgFd);
    } else {
        char noImage[] = " 0";
        sendTCPstring(fdTCP, noImage, strlen(noImage));
    }

    char newLine = '\n';
    sendTCPstring(fdTCP, &newLine, strlen(&newLine));
    free(buffer);
    return fdTCP;
    
    return 0;
}

void receiveQuestionSubmit(int fdTCP) {
    char* buffer;
    char** args;
    int size = BUFFER_SIZE;

    recvTCPline(fdTCP, &buffer,&size);

    args = tokenize(buffer);
    stripnewLine(args[1]);
    if(!strcmp(args[1], "OK"))
        printf("Question successfully submited\n");
    else if(!strcmp(args[1], "NOK"))
        printf("Something went wrong please try again\n");
    else if(!strcmp(args[1], "DUP"))
        printf("Question already exists in selected topic\n");
    else if (!strcmp(args[1], "FUL"))
        printf("Selected topic already has max number of questions\n");
    else if(!strcmp(args[1], "ERR"))
        printf(SERVER_ERROR);

    close(fdTCP);
    return;

}
