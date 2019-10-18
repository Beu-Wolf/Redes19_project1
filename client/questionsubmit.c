#include "clientcommands.h"

#define FINFO_BUFF_SIZE 50 

void processQuestionSubmit(char** parsedInput, char** questionList) {
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
    if(!isValidQuestion(parsedInput[1])) {
        fprintf(stderr, QUESTION_ERROR);
        return;
    }

    // check if readable files
    if(access(parsedInput[2], R_OK)) {
        fprintf(stderr, FILE_NOT_AVAILABLE_ERROR);
        return;
    }

    if(len == 4 && access(parsedInput[3], R_OK)) {
        fprintf(stderr, IMAGE_NOT_AVAILABLE_ERROR);
        return;
    }

    fdTCP = sendQuestionSubmit(parsedInput);

    if(fdTCP == -1)
        return;

    receiveQuestionSubmit(fdTCP, parsedInput, questionList);
    close(fdTCP);
}

int sendQuestionSubmit(char** parsedInput) {
    long questionFileSz, imgSz;
    int fdTCP, n;
    char* buffer;
    char* imgExt;
    FILE* imgFd;
    FILE* questionFD; 
    char hasImg;

    // prep data

    if ((hasImg = arglen(parsedInput) == 4)) {
        imgFd = fopen(parsedInput[3], "r");
        if(imgFd == NULL)
            fatal(FILEREAD_ERROR);

        // get img extension
        strtok(parsedInput[3], ".");
        imgExt = strtok(NULL, ".");
        if(imgExt == NULL || strlen(imgExt) != 3) {
            printf(INVALID_QS_IMGEXT);
            fclose(imgFd);
            return -1;
        }
        imgSz = fileSize(imgFd);
    }

    questionFD = fopen(parsedInput[2], "r");
    if(questionFD == NULL)
        fatal(FILEREAD_ERROR);

    // get file size
    fseek(questionFD, 0, SEEK_END);
    questionFileSz = ftell(questionFD);
    fseek(questionFD, 0L, SEEK_SET);

    fdTCP = socket(tcpInfo->ai_family, tcpInfo->ai_socktype, tcpInfo->ai_protocol);
    if(fdTCP == -1) fatal(SOCK_CREATE_ERROR);

    setSocketTimeout(fdTCP, CLIENT_TIMEOUT);

    n = connect(fdTCP, tcpInfo->ai_addr, tcpInfo->ai_addrlen);
    if(n == -1) fatal(SOCK_CONN_ERROR);


    // send request
      // 1: send until file data
    buffer = (char*) malloc(sizeof(char)*FINFO_BUFF_SIZE);
    if(!buffer) fatal(ALLOC_ERROR);
    memset(buffer, 0, FINFO_BUFF_SIZE);

    sprintf(buffer, "QUS %s %s %s %ld ", userID, selectedTopic, 
        parsedInput[1], questionFileSz);
    sendTCPstring(fdTCP, buffer, strlen(buffer));

      // 2: send file data
    sendTCPfile(fdTCP, questionFD);

      // 3
    if(hasImg) {
        memset(buffer, 0, FINFO_BUFF_SIZE);
        sprintf(buffer, " %d %s %ld ", 1, imgExt, imgSz);
        sendTCPstring(fdTCP, buffer, strlen(buffer));

        sendTCPfile(fdTCP, imgFd);
        fclose(imgFd);
    } else {
        char noImage[] = " 0";
        sendTCPstring(fdTCP, noImage, strlen(noImage));
    }

    char newLine = '\n';
    sendTCPstring(fdTCP, &newLine, strlen(&newLine));
    free(buffer);
    fclose(questionFD);
    return fdTCP;
}

void receiveQuestionSubmit(int fdTCP, char** parsedInput, char**questionList) {
    char* buffer;
    char** args;
    int size = BUFFER_SIZE;

    recvTCPline(fdTCP, &buffer,&size);
    args = tokenize(buffer);

    if(!strcmp(args[0], "ERR")) {
        printf(SERVER_ERROR);
        close(fdTCP);
        return;
    }

    if(arglen(args) < 2) {
        close(fdTCP);
        return;
    }

    stripnewLine(args[1]);
    if(!strcmp(args[1], "OK")){
        printf("Question successfully submited\n");
        int i;
        for(i = 0; questionList[i] != 0; i++);
        questionList[i] = strdup(parsedInput[1]);
        if(questionList[i] == NULL) fatal(STRDUP_ERROR);
        selectedQuestion = questionList[i];
    }
    else if(!strcmp(args[1], "NOK"))
        printf("Something went wrong please try again\n");
    else if(!strcmp(args[1], "DUP"))
        printf("Question already exists in selected topic\n");
    else if (!strcmp(args[1], "FUL"))
        printf("Selected topic already has max number of questions\n");


    close(fdTCP);
    free(buffer);
    free(args);
    return;
}
