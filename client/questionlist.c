#include "clientcommands.h"

void processQuestionList(int fdUDP, char **args, char **questionList) {
    if (!isRegistered()) {
        fprintf(stderr, NOT_REGISTERED_ERROR);
        return;
    }

    if (arglen(args) != 1) {
        fprintf(stderr, INVALID_QL_ARGS);
        return;
    }

    if (selectedTopic == NULL) {
        fprintf(stderr, NO_TOPIC_SELECTED_ERROR);
        return;
    }

    sendQuestionList(fdUDP);
    receiveQuestionList(fdUDP, questionList);
}

void sendQuestionList(int fdUDP) {
    char *sendMsg = strdup("LQU ");

    sendMsg = safestrcat(sendMsg, selectedTopic);
    sendMsg = safestrcat(sendMsg, "\n");

    if(sendto(fdUDP, sendMsg, strlen(sendMsg), 0, udpInfo->ai_addr, udpInfo->ai_addrlen) == -1)
        fatal(strerror(errno));

    free(sendMsg);
}

void receiveQuestionList(int fdUDP, char **questionList) {
    char buffer[BUFFER_SIZE];
    char question[16];
    char userID[8];
    char numAnswers[4];
    int questionNum;
    char **args;
    char **questions;

    if(recvfrom(fdUDP, buffer, BUFFER_SIZE, 0, NULL, NULL) == -1)
        fatal(UDPRECV_ERROR);

    stripnewLine(buffer);
    args = tokenize(buffer);

    if (arglen(args) < 2) {
        printf("Something went wrong. Please try again\n");
        free(args);
        return;
    }

    if (isPositiveNumber(args[1])) {
        questionNum = strtol(args[1], NULL, 10);
    } else {
        printf("Invalid Number of questions\n");
        free(args);
        return;
    }

    if(!strcmp(args[1], "0")) {
        printf("No questions to show in selected topic\n");
        free(args);
        return;
    }

    if(selectedQuestion) selectedQuestion = strdup(selectedQuestion);
    resetPtrArray(questionList, MAXQUESTIONS + 1);
    questions = &(args[2]);
    for (int i = 0; i < questionNum; i++) {
        if (sscanf(questions[i], "%[0-9a-zA-Z]:%[0-9]:%[0-9]", question, userID, numAnswers) == 3) {
            questionList[i] = strdup(question);
            if(selectedQuestion && !strcmp(selectedQuestion, questionList[i])) {
                free(selectedQuestion);
                selectedQuestion = questionList[i];
            }
            printf("%d: %s (%s answers) by %s\n", i, question, numAnswers, userID);
        } else {
            printf("Error while parsing questions. Please try again\n");
            free(args);
            return;
        }
    }
    
    free(args);
}
