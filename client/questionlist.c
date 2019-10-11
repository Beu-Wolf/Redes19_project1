#include "clientcommands.h"

void processQuestionList(int fdUDP, char **args,
        addressInfoSet newAddrInfoSet, char **questionList) {

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

    sendQuestionList(fdUDP, newAddrInfoSet);
    receiveQuestionList(fdUDP, questionList);
}

void sendQuestionList(int fdUDP, addressInfoSet newAddrInfoSet) {
    char *sendMsg = strdup("LQU ");

    sendMsg = safestrcat(sendMsg, selectedTopic);
    sendMsg = safestrcat(sendMsg, "\n");

    if(sendto(fdUDP, sendMsg, strlen(sendMsg), 0,
            newAddrInfoSet.res_UDP->ai_addr,
            newAddrInfoSet.res_UDP->ai_addrlen) == -1) fatal(strerror(errno));

    free(sendMsg);
}

void receiveQuestionList(int fdUDP, char **questionList) {
    int questionNum;
    char **args;

    char buffer[BUFFER_SIZE]; // TODO: depends on max number of questions

    if(recvfrom(fdUDP, buffer, BUFFER_SIZE, 0, NULL, NULL) == -1)
        fatal(UDPRECV_ERROR);

    stripnewLine(buffer);
    args = tokenize(buffer);

    if (arglen(args) < 2) {
        // TODO: handle wrong number of arguments
        printf("Something went wrong. Please try again\n");
        questionList = NULL; // terminate question list
        free(args);
        return;

    }
    

    if (isPositiveNumber(args[1])) {
        questionNum = strtol(args[1], NULL, 10);
    } else {
        // TODO: handle invalid number of questions
        printf("Invalid Number of questions\n");
        questionList = NULL; // terminate question list
        free(args);
        return;
    }

    if(!strcmp(args[1], "0")) {
        printf("No questions to show in selected topic\n");
        questionList = NULL; // terminate question list

        free(args);
        return;
    }

    char** questionListBegin = questionList;

    char **questions = args + 2;
    char question[16];
    char userID[16];
    char numAnswers[8];

    for (int i = 0; i < questionNum; i++) {
        if (sscanf(questions[i], "%[0-9a-zA-Z]:%[0-9]:%[0-9]",
                    question, userID, numAnswers) == 3) {

            *questionList = strdup(question);
            questionList++;

            printf("%d: %s (%s answers) by %s\n",
                    i, question, numAnswers, userID);
        } else {
            // TODO: handle invalid question:user:ans sequence
            printf("Error while parsing questions. Please try again\n");
            questionList = NULL; // terminate question list

            free(args);
            return;
        }
    }

    
    questionList = questionListBegin;


    free(args);
}
