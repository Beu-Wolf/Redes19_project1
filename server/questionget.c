#include "commands.h"
#define FINFO_BUFF_SIZE 50

static char *question;
static char *topic;

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

char* getImageInfo(char* p) {
    char* cleaner;                                   //to consume userID in first line of datafile
    char* imageExt;
    char* path = strdup(p);
    path = safestrcat(path, "/"DATAFILE);


    FILE* dataFile = fopen(path, "r");
    if(!dataFile) fatal(FILE_NOT_AVAILABLE_ERROR);
    if(fscanf(dataFile, "%ms %ms", &cleaner, &imageExt) < 2) {
        imageExt = NULL;
    }

    free(cleaner);
    fclose(dataFile);
    free(path);


    return imageExt;

}

void processQuestionGet(int fdTCP) {
    char *buffer;
    int size;
    char errorResponses[9];
    long questionFileSize;

    //Read topic and check existance
    size = 11;
    topic = getTopic(fdTCP, size);
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

    questionFileSize = fileSize(questionFD);

    //send 1st part (protocol, ID, size)
    buffer = (char*) malloc(sizeof(char)*FINFO_BUFF_SIZE);
    if(!buffer) fatal(ALLOC_ERROR);
    memset(buffer, 0, FINFO_BUFF_SIZE);

    sprintf(buffer, "QGR %s %ld ", user, questionFileSize);
    sendTCPstring(fdTCP, buffer, strlen(buffer));

    //send question file
    sendTCPfile(fdTCP, questionFD);

    fclose(questionFD);

    char* imageExt;

    imageExt = getImageInfo(path);
    if(imageExt != NULL) {
        char imageBuffer[BUFFER_SIZE];
        char *imagePath = strdup(path);
        FILE *imageFile;
        long imageSize;

        imagePath = safestrcat(imagePath, "/");
        imagePath = safestrcat(imagePath, question);
        imagePath = safestrcat(imagePath, ".");
        imagePath = safestrcat(imagePath, imageExt);

        imageFile = fopen(imagePath, "r");
        if (!imageFile) {
            /* TODO: What do we do in this situation? */
            fprintf(stderr, "Error opening %s\n", imagePath);
        }

        imageSize = fileSize(imageFile);

        snprintf(imageBuffer, BUFFER_SIZE, " 1 %s %ld ", imageExt, imageSize);
        sendTCPstring(fdTCP, imageBuffer, strlen(imageBuffer));

        sendTCPfile(fdTCP, imageFile);

        free(imagePath);
        free(imageExt);
    } else {
        sendTCPstring(fdTCP, " 0", 2);
    }

}
