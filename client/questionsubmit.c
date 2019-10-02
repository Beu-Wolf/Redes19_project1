#include "clientcommands.h"

void processQuestionSubmit(char** parsedInput, addressInfoSet newAddrInfoSet) {
    int len = arglen(parsedInput);
    

    if(!isRegistered()) {
        printf(NOT_REGISTERED_ERROR);
        return;
    }

    if(selectedTopic == NULL) {
        printf(NO_TOPIC_SELECTED_ERROR);
        return;
    } 

    if(len < 3 || len > 4){
        printf(INVALID_QS_ARGS);
        return;
    }

    if(strlen(parsedInput[1]) > QUESTION_MAXLEN) {
        printf(QSIZE_ERROR);
        return;
    }

    if(access(parsedInput[2], R_OK) || (len == 4 && access(parsedInput[3], R_OK))) {
        printf(FILE_NOT_AVAILABLE_ERROR);
        return;
    }


    // sendQuestionSubmit(parsedInput, newAddrInfoSet);
    // receiveQuestionSubmit(fdTCP);
}

void sendQuestionSubmit(char** parsedInput, addressInfoSet newAddrInfoSet) {
    
    return;
    /* char* questionFile, *imageFile;
    long textfileSize, imagefileSize = 0;
    int n;
    char* question = parsedInput[1];
    char* imageExtension;

    char* buffer;

    FILE* imageFilePointer;

    FILE* questionFilePointer = fopen(parsedInput[2], "r");
    if(questionFilePointer == NULL) {
        fatal(FILEREAD_ERROR);
    }

    fseek(questionFilePointer, 0, SEEK_END);
    textfileSize = ftell(questionFilePointer);

    printf("size: %ld\n", textfileSize);

    fseek(questionFilePointer, 0L, SEEK_SET);
    questionFile = (char*) malloc(sizeof(char) * textfileSize);
    if(!questionFile) fatal(ALLOC_ERROR);

    memset(questionFile, 0, sizeof(char) * textfileSize);

    fread(questionFile, sizeof(char), textfileSize, questionFilePointer);

    // printf("%s\n", questionFile);

    if(hasImage){
        imageFilePointer = fopen(parsedInput[3], "r");
        if(imageFilePointer == NULL) {
            fatal(FILEOPEN_ERROR);
        }

        strtok(parsedInput[3], ".");
        imageExtension = strtok(NULL, ".");

        if(imageExtension == NULL) {
            printf(INVALID_QS_IMGEXT);
            return;
        }

        imagefileSize = fseek(imageFilePointer, 0, SEEK_END);
        fseek(imageFilePointer, 0, SEEK_SET);
        imageFile = (char*) malloc(sizeof(char) * imagefileSize);
        if(!imageFile) fatal(ALLOC_ERROR);

        memset(imageFile, 0, sizeof(char) * imagefileSize);

        fread(imageFile, sizeof(char), imagefileSize, imageFilePointer);
    }

    if(!hasImage) {
        buffer = (char*) malloc(sizeof(char) * (textfileSize + BUFFER_SIZE));
        if(!buffer) fatal(ALLOC_ERROR);


        memset(buffer, 0, sizeof(char) * (textfileSize + BUFFER_SIZE));
        
        sprintf(buffer, "QUS %d %s %s %ld %s %d\n", userID, selectedTopic, parsedInput[1], textfileSize,
          questionFile, hasImage);
        printf("%s", buffer);

    } else {
        buffer = questionFile = (char*) malloc(sizeof(char) * (textfileSize + imagefileSize + BUFFER_SIZE));
        if(!buffer) fatal(ALLOC_ERROR);

        memset(buffer, 0, sizeof(char) * (textfileSize + imagefileSize + BUFFER_SIZE));

        sprintf(buffer, "QUS %d %s %s %ld %s %d %s %ld %s\n", userID, selectedTopic, 
        parsedInput[1], textfileSize, questionFile, hasImage, imageExtension, 
        imagefileSize, imageFile);

        printf("%s", buffer);
    }

    fdTCP = socket(newAddrInfoSet.res_TCP->ai_family,
      newAddrInfoSet.res_TCP->ai_socktype, newAddrInfoSet.res_TCP->ai_protocol);
    if(fdTCP == -1) fatal(SOCK_CREATE_ERROR);

    n = connect(fdTCP, newAddrInfoSet.res_TCP->ai_addr,
      newAddrInfoSet.res_TCP->ai_addrlen);
    if(n == -1) fatal(SOCK_CONN_ERROR);

    sendTCPstring(fdTCP, buffer);

    free(questionFile);
    free(buffer);
    if(hasImage){
        free(imageFile);
        fclose(imageFilePointer);
    }    
    fclose(questionFilePointer);
    */
}

void receiveQuestionSubmit(int fdTCP) {
    char** buffer;
    int size = BUFFER_SIZE;

    return;
    recvTCPline(fdTCP, buffer,&size);

    printf("%s", *buffer);
    close(fdTCP);

}
