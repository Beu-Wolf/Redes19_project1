#include "commands.h"
#include <time.h>
#include <stdbool.h>

static int receiveAnswer(int fd);
static int numAnswers(char *path);
static int storeAnswer(int fd, char *path);

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

    /* TODO: remove */
    printf("uid: %s\n", userID);
    printf("topic: %s\n", topic);
    printf("question: %s\n", question);
    printf("asize: %s\n", asize);

    /* TODO: validate everything */

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

     n = numAnswers(path);
     if (n == ERROR) return n;

     if (n == MAXANSWERS) {
         fprintf(stderr, "The question is already full!\n");
         free(path);
         return FULL;
     }

     sprintf(tmp, "/answer_%02d", n+1);
     path = safestrcat(path, tmp);

     n = mkdir(path, 0755);
     if (n == -1) {
         fprintf(stderr, "Error creating answer directory!\n");
         free(path);
         return ERROR;
     }

     n = storeAnswer(fd, path);
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

static int storeAnswer(int fd, char *path) {
    char *answer = strdup(path);
    char *data = strdup(path);
    char *image = strdup(path);
    char *aimg;
    char tmp[20];
    long fileSize;
    bool error = false;
    int n;

    answer = safestrcat(answer, "/"ANSWERFILE);
    data = safestrcat(data, "/"DATAFILE);
    image = safestrcat(image, "/"IMAGEFILE);

    FILE *answerFile = fopen(answer, "w");
    FILE *dataFile = fopen(data, "w");

    fileSize = toNonNegative(asize);
    recvTCPfile(fd, fileSize, answerFile);

    fprintf(dataFile, "%s", userID);

    /* Absorb space between answer file and image flag*/
    n = recv(fd, tmp, 1, 0);
    if (n == 0 || *tmp != ' ') {
        error = true;
        goto clean;
    }

    recvTCPword(fd, &aimg, NULL);
    if (!strcmp(aimg, "1")) {
        FILE *imageFile = fopen(image, "w");
        char *isize;
        char *iext;
        long imageSize;

        recvTCPword(fd, &iext, NULL);
        fprintf(dataFile, " %s", iext);

        recvTCPword(fd, &isize, NULL);
        imageSize = toNonNegative(isize);

        recvTCPfile(fd, imageSize, imageFile);

        fclose(imageFile);
        free(isize);
        free(iext);
    } else if (strcmp(aimg, "0")) { /* not "0" either */
        error = true;
        goto clean;
    }

    /* Absorb newline */
    n = recv(fd, tmp, 1, 0);
    if (n == 0 || *tmp != '\n') {
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
