#include "commands.h"
#define FINFO_BUFF_SIZE 50 

static char *questionUser(char *p) {
    char *user;
    char *path = strdup(p);
    path = safestrcat(path, "/"DATAFILE);

    FILE *datafile = fopen(path, "r");
    fscanf(datafile, "%ms", &user);

    fclose(datafile);
    free(path);

    return user;
}

char* getImageInfo(char* p, char* imageFlag) {
    char* cleaner;                                   //to consume userID in first line of datafile
    char* imageExt;                     
    char* path = strdup(p);
    path = safestrcat(path, "/"DATAFILE);


    FILE* dataFile = fopen(path, "r");
    if(!dataFile) fatal(FILE_NOT_AVAILABLE_ERROR);
    fscanf(dataFile, "%ms\n %c %ms", &cleaner, imageFlag, &imageExt);

    fclose(dataFile);
    free(path);
    if(!strcmp(imageFlag, "0")) {
        return NULL;
    }

    return imageExt;
    
}

void processQuestionGet(int fdTCP) {
    char* topic, *question, *buffer;
    int size;
    char errorResponses[9];
    long questionFileSize;

    //Read topic and check existance
    size = 11;
    topic  = getTopic(fdTCP, size);
    if(!topic) {
        strcpy(errorResponses, "QGR ERR\n");                  //send error message
        sendTCPstring(fdTCP, errorResponses, strlen(errorResponses));
        return;
    }

    char path[BUFFER_SIZE];
    strcpy(path, TOPICSDIR"/");
    strcat(path, topic);

    DIR* topicDirp = opendir(path);
    if(!topicDirp) {
        strcpy(errorResponses, "QGR EOF\n");                  //send no topic/question error message
        sendTCPstring(fdTCP, errorResponses, strlen(errorResponses));
        return;
    }


    //Read question and get existance
    size = 11;
    question = getQuestion(fdTCP, size);

    if(!question) {
        strcpy(errorResponses, "QGR ERR\n");                  //send error message
        sendTCPstring(fdTCP, errorResponses, strlen(errorResponses));
        return;
    }
    strcat(path, "/");
    strcat(path, question);
    DIR* questionDirp = opendir(path);
    if(!questionDirp) {
        strcpy(errorResponses, "QGR EOF\n");                  //send no topic/question error message
        sendTCPstring(fdTCP, errorResponses, strlen(errorResponses));
        return;
    }

    char* user = questionUser(path);

    //get question.txt
    char* questionFile = strdup(path);
    questionFile = safestrcat(questionFile, "/");
    questionFile = safestrcat(questionFile, question);
    questionFile = safestrcat(questionFile, ".txt");

    FILE* questionFD = fopen(questionFile, "r");
    if(!questionFD) {
        strcpy(errorResponses, "QGR ERR\n");                  //send error message
        sendTCPstring(fdTCP, errorResponses, strlen(errorResponses));
        return;
    }

    //get file size
    fseek(questionFD, 0, SEEK_END);
    questionFileSize = ftell(questionFD);
    fseek(questionFD, 0L, SEEK_SET);

    //send 1st part (protocol, ID, size)
    buffer = (char*) malloc(sizeof(char)*FINFO_BUFF_SIZE);
    if(!buffer) fatal(ALLOC_ERROR);
    memset(buffer, 0, FINFO_BUFF_SIZE);

    sprintf(buffer, "QGR %s %ld ", user, questionFileSize);
    sendTCPstring(fdTCP, buffer, strlen(buffer));
    
    //send question file
    sendTCPfile(fdTCP, questionFD);

    fclose(questionFD);

    char imageFlag;
    char* imageExt;

    imageExt = getImageInfo(path, &imageFlag);
    if(imageExt != NULL)
        printf("%s\n", imageExt);

    


}