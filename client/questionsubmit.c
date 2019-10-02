#include "clientcommands.h"

void processQuestionSubmit(char** parsedInput, addressInfoSet newAddrInfoSet) {

    int hasImage = 0;
    int len =  arglen(parsedInput);
    int fdTCP;
    

    if(len < 3 || len > 4){
        printf(INVALID_QS_ARGS);
        return;
    } else if (len == 4) {
        hasImage = 1;
    }

    if(selectedTopic == NULL) {
        printf("Can't submit question without selecting a topic\n");
        return; 
    } 

    sendQuestionSubmit(&fdTCP, parsedInput, newAddrInfoSet, hasImage);
    //receiveQuestionSubmit(&fdTCP);

}

void sendQuestionSubmit(int* fdTCP, char** parsedInput, addressInfoSet newAddrInfoSet,
 int hasImage){

    char* questionFile, *imageFile;
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
    }

    buffer = (char*) malloc(sizeof(char) * BUFFER_SIZE);
    if(!buffer) exit(1);


    memset(buffer, 0, sizeof(char) * BUFFER_SIZE);
    
    sprintf(buffer, "QUS %d %s %s %ld ", userID, selectedTopic, parsedInput[1], textfileSize);
    printf("%s\n", buffer);

    *fdTCP = socket(newAddrInfoSet.res_TCP->ai_family,
      newAddrInfoSet.res_TCP->ai_socktype, newAddrInfoSet.res_TCP->ai_protocol);
    if(*fdTCP == -1) fatal(SOCK_CREATE_ERROR);

    n = connect(*fdTCP, newAddrInfoSet.res_TCP->ai_addr,
      newAddrInfoSet.res_TCP->ai_addrlen);
    if(n == -1) fatal(SOCK_CONN_ERROR);

    sendTCPstring(*fdTCP, buffer);
    printf("Sent TCP string\n");

    memset(buffer, 0, strlen(buffer));

    sendTCPfile(*fdTCP, questionFilePointer);

    if(!hasImage){
        sprintf(buffer, "%d\n", hasImage);
        sendTCPstring(*fdTCP, buffer);
    } else {
        sprintf(buffer, "%d %s %ld ", hasImage, imageExtension, imagefileSize);
        sendTCPstring(*fdTCP, buffer);
        sendTCPfile(*fdTCP, imageFilePointer);
        char newLine = '\n';
        write(*fdTCP, &newLine, sizeof(newLine));
    }


    free(buffer);
    if(hasImage){
        fclose(imageFilePointer);
    }    
    fclose(questionFilePointer);

    close(*fdTCP);

}

void receiveQuestionSubmit(int* fdTCP) {
    char* buffer;
    int size = BUFFER_SIZE;
    recvTCPline(*fdTCP, &buffer,&size);

    printf("%s", buffer);
    close(*fdTCP);

}
