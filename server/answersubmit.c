#include "commands.h"
#include <time.h>
#include <stdbool.h>

static int receiveAnswer(int fd);
static int numAnswers(char *path);
static int storeAnswer(int fd, char *path, int n);

static char *userID;
static char *topic;
static char *question;
static char *asize;

#define FULL -2
#define ERROR -1
#define OK 0

void processAnswerSubmit(int fd) {
    int n;
    char *response = strdup("ANR ");

    recvTCPword(fd, &userID, NULL);
    recvTCPword(fd, &topic, NULL);
    recvTCPword(fd, &question, NULL);
    recvTCPword(fd, &asize, NULL);

    n = receiveAnswer(fd);
    if (n == ERROR) {
        response = safestrcat(response, "ERR\n");
    } else if (n == FULL) {
        response = safestrcat(response, "FUL\n");
    } else {
        response = safestrcat(response, "OK\n");
    }

    sendTCPstring(fd, response, strlen(response));

    free(userID);
    free(topic);
    free(question);
    free(asize);

    return;
}

static int receiveAnswer(int fd) {
    char *path = strdup(TOPICSDIR"/");
    char tmp[BUFFER_SIZE];
    int n;
    int num;

    path = safestrcat(path, topic);

    if (access(path, F_OK) != 0) {
        fprintf(stderr, "Topic %s doesn't exist!\n", topic);
        return ERROR;
    }

    path = safestrcat(path, "/");
    path = safestrcat(path, question);

    if (access(path, F_OK) != 0) {
        fprintf(stderr, "Question %s doesn't exist in topic %s!\n",
                question, topic);
        free(path);
        return ERROR;
    }

    /* Lock question to avoid answers with the same number */
    questionLock(topic, question);

    n = numAnswers(path);
    num = n + 1;
    if (n == ERROR) return n;
        

    if (n >= MAXANSWERS) {
        fprintf(stderr, "The question is already full!\n");
        free(path);
        return FULL;
    }

    sprintf(tmp, "/%s_%02d", question, n+1);
    path = safestrcat(path, tmp);

    n = mkdir(path, 0755);
    if (n == -1) {
        fprintf(stderr, "Error creating answer directory!\n");
        free(path);
        return ERROR;
    }

    questionUnlock();

    n = storeAnswer(fd, path, num);
    if (n == ERROR){
        free(path);
        return n;
    }

    free(path);
    return OK;
}

static int numAnswers(char *path) {
    struct dirent *questionEnt;
    DIR *question;
    int count;

    question = opendir(path);
    if (question == NULL) {
        fprintf(stderr, "Error opening %s\n", path);
        return ERROR;
    }

    count = 0;
    while ((questionEnt = readdir(question))) {
        char *name = questionEnt->d_name;

        if (strcmp(name, ".")
                && strcmp(name, "..")
                && questionEnt->d_type == DT_DIR) {
            count++;
        }
    }

    closedir(question);
    return count;
}

static int storeAnswer(int fd, char *path, int num) {
    char *answer = strdup(path);
    char *data = strdup(path);
    char *image = strdup(path);
    char tmp[20];
    long fileSize;
    bool error = false;
    int ret;

    answer = safestrcat(answer, "/");
    sprintf(tmp, "%s_%02d", question, num);
    answer = safestrcat(answer, tmp);
    answer = safestrcat(answer, ".txt");
    data = safestrcat(data, "/"DATAFILE);
    image = safestrcat(image, "/");
    image = safestrcat(image, tmp);

    FILE *answerFile = fopen(answer, "w");
    FILE *dataFile = fopen(data, "w");

    fileSize = toNonNegative(asize);
    recvTCPfile(fd, fileSize, answerFile);

    fprintf(dataFile, "%s", userID);

    /* Absorb space between answer file and image flag*/
    ret = recvTCPchar(fd, tmp);
    if (ret == 0 || *tmp != ' ') {
        error = true;
        goto clean;
    }

    ret = recvTCPchar(fd, tmp);
    if (ret == 1 && *tmp == '1') {
        FILE *imageFile;
        char *isize;
        char *iext;
        long imageSize;

        /* Absorb space between aimg and iext */
        ret = recvTCPchar(fd, tmp);
        if (ret == 0 || *tmp != ' ') {
            error = true;
            goto clean;
        }

        imageFile = fopen(image, "w");

        recvTCPword(fd, &iext, NULL);
        fprintf(dataFile, " %s", iext);

        image = safestrcat(image, ".");
        image = safestrcat(image, iext);

        imageFile = fopen(image, "w");

        recvTCPword(fd, &isize, NULL);
        imageSize = toNonNegative(isize);

        recvTCPfile(fd, imageSize, imageFile);

        fclose(imageFile);
        free(isize);
        free(iext);
    } else if (ret != 1 || *tmp != '0') {
        error = true;
        goto clean;
    }

    /* Absorb newline */
    ret = recvTCPchar(fd, tmp);
    if (ret == 0 || *tmp != '\n') {
        error = true;
        goto clean;
    }

    clean:
    free(answer);
    free(data);
    free(image);
    fclose(answerFile);
    fclose(dataFile);

    return error ? ERROR : OK;
}
