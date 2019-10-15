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
    DIR *dir = opendir(p);
    struct dirent *answerEnt;
    int answerCount = 0;

    while ((answerEnt = readdir(dir))) {
        char *answer = answerEnt->d_name;

        if (strcmp(answer, ".")
                && strcmp(answer, "..")
                && strcmp(answer, DATAFILE)
                && answerEnt->d_type == DT_DIR) {
            answerCount++;
        }
    }

    closedir(dir);

    return answerCount;
}

char *processQuestionList(char **args) {
    char *questionListStatus = (char *)malloc(BUFFER_SIZE * sizeof(char));
    char *newLine = "\n";
    if (!questionListStatus) exit(1);

    if (arglen(args) != 2) {
        strncpy(questionListStatus, "ERR\n", 5);
        return questionListStatus;
    }

    char *topic = args[1];
    stripnewLine(topic);

    char topicPath[BUFFER_SIZE];
    strncpy(topicPath, TOPICSDIR"/", strlen(TOPICSDIR) + 3);
    strcat(topicPath, topic);

    DIR *topicDir = opendir(topicPath);
    if (topicDir == NULL) exit(1);

    int questionCount = 0;
    struct dirent *questionEnt;

    char *info = strdup(""); // space after "LQR N"

    while ((questionEnt = readdir(topicDir))) {
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

        sprintf(tmp, " %s:%s:%d", question, user, answers);
        info = safestrcat(info, tmp);

        free(user);
        free(questionPath);
    }

    sprintf(questionListStatus, "LQR %d", questionCount);

    if (questionCount != 0){
        questionListStatus = safestrcat(questionListStatus, info);
        questionListStatus = safestrcat(questionListStatus, newLine);
    } else    
        questionListStatus = safestrcat(questionListStatus, newLine);

    free(info);
    closedir(topicDir);

    return questionListStatus;
}
