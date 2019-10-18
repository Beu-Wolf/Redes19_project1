#include "commands.h"
#define FINFO_BUFF_SIZE 50

static char *question;
static char *topic;

static void sendAnswers(int fd, char *path);
static void sendAnswerData(int fd, char *path, char *answerName);
static void sendAnswerFile(int fd, char *file);

static int answerCmp(const void *a, const void *b);

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
    char* cleaner;  //to consume userID in first line of datafile
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
        strncpy(errorResponses, "QGR ERR\n", 9);                  //send error message
        sendTCPstring(fdTCP, errorResponses, strlen(errorResponses));
        return;
    }

    char path[BUFFER_SIZE];
    strncpy(path, TOPICSDIR"/", 13);
    strcat(path, topic);

    DIR* topicDirp = opendir(path);
    if(!topicDirp) {
        strncpy(errorResponses, "QGR EOF\n", 9);                  //send no topic/question error message
        sendTCPstring(fdTCP, errorResponses, strlen(errorResponses));
        return;
    }

    //Read question and get existance
    size = 11;
    question = getQuestion(fdTCP, size);

    questionUnlock();

    if(!question) {
        strncpy(errorResponses, "QGR ERR\n", 9);                  //send error message
        sendTCPstring(fdTCP, errorResponses, strlen(errorResponses));
        return;
    }
    strcat(path, "/");
    strcat(path, question);
    DIR* questionDirp = opendir(path);
    if(!questionDirp) {
        strncpy(errorResponses, "QGR EOF\n", 9);                  //send no topic/question error message
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
        strncpy(errorResponses, "QGR ERR\n", 9);                  //send error message
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

    sendAnswers(fdTCP, path);
    sendTCPstring(fdTCP, "\n", 1);

    questionUnlock();
}

static void sendAnswers(int fd, char *path) {
    DIR *questionDir = opendir(path);
    struct dirent *answer;
    char tmp[BUFFER_SIZE];

    char *answers[MAXANSWERS+1] = {0};

    if (questionDir == NULL) {
        fprintf(stderr, "Error opening directory %s\n", path);
    }

    int numAnswers = 0;
    while ((answer = readdir(questionDir))) {
        char *name = answer->d_name;
        if (!strcmp(name, ".")
                || !strcmp(name, "..")
                || answer->d_type != DT_DIR) {
            continue;
        }

        answers[numAnswers] = strdup(name);
        numAnswers++;
    }

    qsort(answers, numAnswers, sizeof(char*), answerCmp);

    sprintf(tmp, " %d", MIN(numAnswers, 10));
    sendTCPstring(fd, tmp, strlen(tmp));

    int lim = MIN(numAnswers, 10);
    for (int i = 0; i < lim; i++) {
        sendAnswerData(fd, path, answers[i]);
    }

    for (int i = 0; i < numAnswers; i++) {
        free(answers[i]);
    }
}

static void sendAnswerData(int fd, char *path, char *answerName) {
    char *answerPath = strdup(path);
    char answerUser[8];
    char answerImageExt[8];
    char *answerText;
    char *answerImage;
    char *answerData;
    char *answerNum;
    char tmp[BUFFER_SIZE];
    int ret;
    bool hasImage = false;
    FILE *answerDataFile;

    answerPath = safestrcat(answerPath, "/");
    answerPath = safestrcat(answerPath, answerName);
    answerPath = safestrcat(answerPath, "/");

    answerData = strdup(answerPath);
    answerData = safestrcat(answerData, DATAFILE);

    answerDataFile = fopen(answerData, "r");
    ret = fscanf(answerDataFile, "%s %s", answerUser, answerImageExt);
    fclose(answerDataFile);

    if (ret == 2) {
        hasImage = true;
    }

    /* The number is the last 2 chars of the string */
    answerNum = answerName + strlen(answerName) - 2;

    sprintf(tmp, " %s %s ", answerNum, answerUser);
    sendTCPstring(fd, tmp, strlen(tmp));

    answerText = strdup(answerPath);
    answerText = safestrcat(answerText, answerName);
    answerText = safestrcat(answerText, ".txt");

    sendAnswerFile(fd, answerText);

    if (hasImage) {
        answerImage = strdup(answerPath);
        answerImage = safestrcat(answerImage, answerName);
        answerImage = safestrcat(answerImage, ".");
        answerImage = safestrcat(answerImage, answerImageExt);

        sprintf(tmp, " 1 %s ", answerImageExt);
        sendTCPstring(fd, tmp, strlen(tmp));

        sendAnswerFile(fd, answerImage);

        free(answerImage);
    } else {
        sendTCPstring(fd, " 0", 2);
    }

    free(answerPath);
    free(answerData);
    free(answerText);
}

/* Sends file size and file contents */
static void sendAnswerFile(int fd, char *path) {
    FILE *file = fopen(path, "r");
    long size = fileSize(file);
    char tmp[20];

    sprintf(tmp, "%ld ", size);
    sendTCPstring(fd, tmp, strlen(tmp));
    sendTCPfile(fd, file);

    fclose(file);
}

static int answerCmp(const void *a, const void *b) {
    char *sa = *(char **)a;
    char *sb = *(char **)b;

    return strcmp(sb, sa);
}
