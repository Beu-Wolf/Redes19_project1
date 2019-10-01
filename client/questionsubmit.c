#include "clientcommands.h"

void processQuestionSubmit(char** parsedInput, addressInfoSet newAddrInfoSet) {

    int hasImage = 0;
    int len =  arglen(parsedInput);
    

    if(len < 3 || len > 4){
        printf(INVALID_QS_ARGS);
        return;
    } else if (len == 4) {
        hasImage = 1;
    }

    if(selectedTopic == NULL) {
        printf("Can't submit question without selecting a topic\n");
    } 

    sendQuestionSubmit(parsedInput, newAddrInfoSet, hasImage);

}

void sendQuestionSubmit(char** parsedInput, addressInfoSet newAddrInfoSet,
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
        printf("Error reading file: %d\n", errno);
        exit(1);
    }

    fseek(questionFilePointer, 0, SEEK_END);
    textfileSize = ftell(questionFilePointer);

    printf("size: %ld\n", textfileSize);

    fseek(questionFilePointer, 0L, SEEK_SET);
    questionFile = (char*) malloc(sizeof(char) * textfileSize);

    if(!questionFile) exit(1);

    memset(questionFile, 0, sizeof(char) * textfileSize);

    fread(questionFile, sizeof(char), textfileSize, questionFilePointer);

    printf("%s\n", questionFile);

    if(hasImage){
        imageFilePointer = fopen(parsedInput[3], "r");
        if(imageFilePointer == NULL) {
            printf("Error reading file: %d\n", errno);
            exit(1);
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
        if(!imageFile) exit(1);


        memset(questionFile, 0, sizeof(char) * imagefileSize);

        fread(imageFile, sizeof(char), imagefileSize, imageFilePointer);
    }

    if(!hasImage) {
        buffer = (char*) malloc(sizeof(char) * (textfileSize + BUFFER_SIZE));
        if(!buffer) exit(1);


        memset(buffer, 0, sizeof(char) * (textfileSize + BUFFER_SIZE));
        
        sprintf(buffer, "QUS %d %s %s %ld %s %d\n", userID, selectedTopic, parsedInput[1], textfileSize,
        questionFile, hasImage);
        printf("%s", buffer);

    } else {
        buffer = questionFile = (char*) malloc(sizeof(char) * (textfileSize + imagefileSize + BUFFER_SIZE));
        if(!buffer) exit(1); 

        memset(buffer, 0, sizeof(char) * (textfileSize + imagefileSize + BUFFER_SIZE));

        sprintf(buffer, "QUS %d %s %s %ld %s %d %s %ld %s\n", userID, selectedTopic, 
        parsedInput[1], textfileSize, questionFile, hasImage, imageExtension, 
        imagefileSize, imageFile);

        printf("%s", buffer);
    }

    int fdTCP = socket(newAddrInfoSet.res_TCP->ai_family,
      newAddrInfoSet.res_TCP->ai_socktype, newAddrInfoSet.res_TCP->ai_protocol);
      if(fdTCP == -1) exit(1);

    n = connect(fdTCP, newAddrInfoSet.res_TCP->ai_addr,
      newAddrInfoSet.res_TCP->ai_addrlen);
    if(n == -1) exit(1);

    sendTCPstring(fdTCP, buffer);




    free(questionFile);
    free(buffer);
    if(hasImage){
        free(imageFile);
        fclose(imageFilePointer);
    }    
    fclose(questionFilePointer);



}