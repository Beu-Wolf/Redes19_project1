#include "commands.h"


void processQuestionSubmit(int fdTCP) {
    char* userId, *topic, *question, *fileSizeStr;
    int size, numQuestions,  n;
    long fileSize;
    char response[9];

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

    char path[BUFFER_SIZE];
    strcpy(path, TOPICSDIR"/");
    strcat(path, topic);                    //Topics folder


    DIR* topicDirp = opendir(path);
    if(!topicDirp) {
        strcpy(response, "ERR\n");                  //send error message
        printf("Fodeu\n");
        sendTCPstring(fdTCP, response);
        return;
    }

    size = 11;                                      //size of question +1
    question = (char*) malloc(sizeof(char) * size);
    if(!question) fatal(ALLOC_ERROR);
    memset(question, 0, 11); 

    recvTCPword(fdTCP, &question, &size);
    printf("Question: |%s|\n", question);

    struct dirent *questionEnt;

    numQuestions = 0;
    while((questionEnt = readdir(topicDirp))) {
        char *currQuestion = questionEnt->d_name;

        if (!strcmp(currQuestion, ".")
                || !strcmp(currQuestion, "..")
                || !strcmp(currQuestion, DATAFILE)) {
            continue;
        }

        if(!strcmp(currQuestion, question)) {
            strcpy(response, "QUR DUP\n"); 
            printf("Duplicado\n");                 //send duplicate message
            sendTCPstring(fdTCP, response);
            closedir(topicDirp);
            return;
        }

        numQuestions++;

        if(numQuestions == 99) {
            strcpy(response, "QUR FUL\n");     
            printf("FULL\n");             //send full message
            sendTCPstring(fdTCP, response);
            closedir(topicDirp);
            return;
        }
    }

    closedir(topicDirp);

    //create folder

    strcat(path, "/");
    strcat(path, question);

    n = mkdir(path, 0700);

    if(n == -1) {
            strcpy(response, "QUR NOK\n");                  //send not ok message
            sendTCPstring(fdTCP, response);
            return;
    }

    //create dataFile
    char* questionFolder = (char*) malloc(sizeof(char)*BUFFER_SIZE);
    strcpy(questionFolder, path);
    questionFolder = safestrcat(questionFolder, "/");
    questionFolder = safestrcat(questionFolder, DATAFILE);

                        

    FILE *questionDatafile = fopen(questionFolder, "w");
    fputs(userId, questionDatafile);
    fclose(questionDatafile);



    //Receive filesize
    size = 11; // size of fileSize plus one
    fileSizeStr = (char*) malloc(sizeof(char) * size);
    if(!fileSizeStr) fatal(ALLOC_ERROR);
    memset(fileSizeStr, 0, 11);

    recvTCPword(fdTCP, &fileSizeStr, &size);
    printf("File Size: |%s|\n", fileSizeStr);

    fileSize = strtol(fileSizeStr, NULL, 0);

    //create question txt

    char* questionFile = (char*) malloc(sizeof(char)*BUFFER_SIZE);
    strcpy(questionFile, path);
    questionFile = safestrcat(questionFile, "/");
    questionFile = safestrcat(questionFile, question);
    questionFile = safestrcat(questionFile, ".txt");

    FILE* questionFilePtr = fopen(questionFile, "w");

    if(!questionFilePtr) {
        strcpy(response, "QUR NOK\n");                  //send not ok message
        sendTCPstring(fdTCP, response);
        return;
    }

    //receive File and write
    recvTCPfile(fdTCP, fileSize, questionFilePtr);

    fclose(questionFilePtr);

    
    

    //9 = max size of protocol mesage for question submit
    // questionSubmitStatus = (char*) malloc(sizeof(char)*9);      
    // if(!questionSubmitStatus) fatal(ALLOC_ERROR);
    // memset(questionSubmitStatus, 0, 9);
    free(fileSizeStr);
    free(questionFolder);
    free(questionFile);
    free(userId);
}
