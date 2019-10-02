#include "commands.h"

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

static int numAnswers(char *p) {
    char *path = strdup(p);
    path = safestrcat(path, "/"ANSWERDIR);

    DIR *answerDir = opendir(path);
    if (answerDir == NULL) {
        if (errno == ENOENT) { // answers/ doesn't exist yet
            return 0;
        } else {
            fatal("Error opening directory");
        }
    }

    struct dirent *answerEnt;
    int answerCount = 0;

    while (answerEnt = readdir(answerDir)) {
        char *answer = answerEnt->d_name;

        if (strcmp(answer, ".")
                && strcmp(answer, "..")
                && strcmp(answer, DATAFILE)) {
            answerCount++;
        }
    }

    free(path);
    closedir(answerDir);

    return answerCount;
}

char *processQuestionList(char **args) {
    char *questionListStatus = (char *)malloc(BUFFER_SIZE * sizeof(char));
    if (!questionListStatus) exit(1);

    if (arglen(args) != 2) {
        strcpy(questionListStatus, "ERR\n");
        return questionListStatus;
    }

    char *topic = args[1];
    stripnewLine(topic);

    char topicPath[BUFFER_SIZE];
    strcpy(topicPath, TOPICSDIR"/");
    strcat(topicPath, topic);

    DIR *topicDir = opendir(topicPath);
    if (topicDir == NULL) exit(1);

    int questionCount = 0;
    struct dirent *questionEnt;

    char *info = strdup(" "); // space after "LQR N"

    while (questionEnt = readdir(topicDir)) {
        char *question = questionEnt->d_name;

        if (!strcmp(question, ".")
                || !strcmp(question, "..")
                || !strcmp(question, DATAFILE)) {
            continue;
        }

        questionCount++;

        char *questionPath = strdup(topicPath);
        questionPath = safestrcat(questionPath, "/");
        questionPath = safestrcat(questionPath, question);

        char *user = questionUser(questionPath);
        int answers = numAnswers(questionPath);
        char tmp[BUFFER_SIZE];

        sprintf(tmp, "%s:%s:%d ", question, user, answers);
        info = safestrcat(info, tmp);

        free(user);
        free(questionPath);
    }

    sprintf(questionListStatus, "LQR %d", questionCount);

    if (questionCount != 0)
        questionListStatus = safestrcat(questionListStatus, info);

    free(info);
    closedir(topicDir);

    return questionListStatus;
}