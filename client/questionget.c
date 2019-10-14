#include "clientcommands.h"

void processQuestionGet(char** parsedInput, char** questionList, addressInfoSet newAddrInfoSet) {
    int wantedNumber, i, fdTCP;
    char abbrev;

    //check if is registered
    if(!isRegistered()) {
        fprintf(stderr ,NOT_REGISTERED_ERROR);
        return;
    }

    //check #args
    if(arglen(parsedInput) != 2) {
        fprintf(stderr, INVALID_QG_ARGS);
        return;
    }

    //check if there is a selected topic
    if(!selectedTopic) {
        fprintf(stderr, NO_TOPIC_SELECTED_ERROR);
        return;
    }


    // check if theres a valid question list client-side
    if(questionList[0] == NULL) {
        fprintf(stderr, NO_QUESTION_LOADED_ERROR);
        return;
   }

    // get question number if qg
    wantedNumber = -1;
    abbrev = strlen(parsedInput[0]) == 2;
    if(abbrev) {
        errno = 0;
        wantedNumber = strtol(parsedInput[1], NULL, 10);
        if(errno != 0 || !isPositiveNumber(parsedInput[1])) {
            printf("Invalid number\n");
            return;
        }
    }

    // get question
    stripnewLine(parsedInput[1]);
    for(i = 0; questionList[i] != 0; i++) {
        if((abbrev && i == wantedNumber) || (!abbrev && !strcmp(parsedInput[1], questionList[i]))) {
            selectedQuestion = questionList[i];
            printf("Selected question: %s\n", selectedQuestion);
            break;
        }

    }

    if(questionList[i] == 0){
        printf("Please select a valid question\n");
        return;
    }

    fdTCP = sendQuestionGet(newAddrInfoSet);

    if(fdTCP == -1)
        return;
    
    /*
    receiveQuestionGet(fdTCP);
    close(fdTCP);
    */
}


int sendQuestionGet(addressInfoSet newAddrInfoSet) {
    int fdTCP, n;
    char* buffer;

    //create socket
    fdTCP = socket(newAddrInfoSet.res_TCP->ai_family, newAddrInfoSet.res_TCP->ai_socktype,
    newAddrInfoSet.res_TCP->ai_protocol);

    if(fdTCP == -1) fatal(SOCK_CREATE_ERROR);

    //connect socket
    n = connect(fdTCP, newAddrInfoSet.res_TCP->ai_addr, newAddrInfoSet.res_TCP->ai_addrlen);
    if(n == -1) fatal(SOCK_CONN_ERROR);

    buffer = (char*) malloc(sizeof(char) * 30);
    if(!buffer) fatal(ALLOC_ERROR);
    memset(buffer, 0, 30);

    sprintf(buffer, "GQU %s %s\n", selectedTopic, selectedQuestion);
    printf("sending: %s", buffer);

    sendTCPstring(fdTCP, buffer, strlen(buffer));
    free(buffer);

    return fdTCP;
}
