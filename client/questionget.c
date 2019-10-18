#include "clientcommands.h"




//Write question file
int writeQuestion(int fdTCP, char* userId, char* path) {
    int n;
    unsigned long long questionFileSize;
    char* question = strdup(path);
    char* image = strdup(path);
    char* qsize;
    char tmp[20];

    memset(tmp, 0, 20);

    question = safestrcat(question, ".txt");
    

    if((n = recvTCPword(fdTCP, &qsize, NULL)) < 1 || n > 10) {
        printf("Error receiving questionFile size\n");
        free(qsize);
        free(question);
        free(image);
        return -1;
    }

    questionFileSize = toNonNegative(qsize);

    FILE* questionFilePtr = fopen(question, "w");
    if(!questionFilePtr) fatal(FILEOPEN_ERROR);

    recvTCPfile(fdTCP, questionFileSize, questionFilePtr); 

    printf("Writing %s by %s\n", question, userId);

    //absorb space between file and image Flag
    n = recvTCPchar(fdTCP, tmp);
    if(n == 0 || *tmp != ' ') {
        free(qsize);
        free(question);
        free(image);
        fclose(questionFilePtr);
        return -1;
    }

    

    n = recvTCPchar(fdTCP, tmp);
    if(n == 1 && *tmp == '1') {
        
        char* iext;
        char* isize;
        long imageFileSize;
        FILE* imageFilePtr;


        /*Absorb space */
        n = recvTCPchar(fdTCP, tmp);
        if(n == 0 || *tmp != ' '){
            free(qsize);
            free(question);
            free(image);
            fclose(questionFilePtr);
            return -1;
        }

        //get extension
        recvTCPword(fdTCP, &iext, NULL);

        image = safestrcat(image, ".");
        image = safestrcat(image, iext);

        imageFilePtr = fopen(image, "w");
        if(!imageFilePtr) fatal(FILEOPEN_ERROR);

        //get size
        recvTCPword(fdTCP, &isize, NULL);
        if(strlen(isize) > 10 || strlen(isize) < 1) {
            printf("Error receiving image File size\n");
            free(qsize);
            free(question);
            free(image);
            fclose(questionFilePtr);
            fclose(imageFilePtr);
            free(isize);
            free(iext);
            return -1;
        }

        imageFileSize = toNonNegative(isize);

        recvTCPfile(fdTCP, imageFileSize, imageFilePtr);

        printf("Writing image: %s by %s\n", image, userId);
        
        fclose(imageFilePtr);

        free(isize);
        free(iext);



    } else if(n != 1 || *tmp != '0') {
        free(qsize);
        free(question);
        free(image);
        fclose(questionFilePtr);
        return - 1;
    }

    free(qsize);
    free(question);
    free(image);
    fclose(questionFilePtr);

    
    return 1;

}

void processQuestionGet(char** parsedInput, char** questionList) {
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

    fdTCP = sendQuestionGet();

    if(fdTCP == -1)
        return;
    
    
    receiveQuestionGet(fdTCP);
    close(fdTCP);
}

int sendQuestionGet() {
    int fdTCP;
    char* buffer;

    //create socket
    fdTCP = socket(tcpInfo->ai_family, tcpInfo->ai_socktype, tcpInfo->ai_protocol);
    if(fdTCP == -1) fatal(SOCK_CREATE_ERROR);

    setSocketTimeout(fdTCP, CLIENT_TIMEOUT);

    //connect socket
    if(connect(fdTCP, tcpInfo->ai_addr, tcpInfo->ai_addrlen) == -1)
        fatal(SOCK_CONN_ERROR);

    buffer = (char*) malloc(sizeof(char) * 30);
    if(!buffer) fatal(ALLOC_ERROR);
    memset(buffer, 0, 30);

    sprintf(buffer, "GQU %s %s\n", selectedTopic, selectedQuestion);

    sendTCPstring(fdTCP, buffer, strlen(buffer));
    free(buffer);

    return fdTCP;
}

void receiveQuestionGet(int fdTCP) {
    char* res = NULL;
    int n;
    long numAnswers, answerNum;

    char* path = NULL;
    char* Nanswers = NULL;
    char* answerN = NULL;
    char* answerFilePath = NULL;

    char tmp[BUFFER_SIZE];
    char space[2];

    char* qUserID, *aUserID;

    recvTCPword(fdTCP, &res, NULL);
    printf("Response: %s\n", res);

    //check protocol flag
    if(strcmp("QGR", res)) {
        fatal(SERVER_ERROR);
    }

    //Get 2nd Argument -> Might be userID or ERR
    recvTCPword(fdTCP, &qUserID, NULL);

    if(!strcmp(qUserID, "EOF\n")) {
        free(res);
        printf("No such topic or question\n");
        return;
    } else if(!strcmp(qUserID, "ERR\n")) {
        free(res);
        printf("Badly formulated request. Please try again\n");
        return;
    } else if(strlen(qUserID) != 5) {
        free(res);
        printf("Wrong question proposer ID\n");
        return;
    }

    path = strdup(selectedTopic);


    //Create topic directory to store answers
    DIR* topicDirp = opendir(path);
    if(ENOENT == errno) {
        n = mkdir(path, 0755);

        if (n == -1) {
            fprintf(stderr, "Error creating topic directory!\n");
            return;
        }
    } else if (!topicDirp) {
        fatal(DIROPEN_ERROR);
    }

    printf("Writing to directory %s/\n", selectedTopic);

    path = safestrcat(path, "/");

    path = safestrcat(path, selectedQuestion);

    if((n = writeQuestion(fdTCP, qUserID, path)) == -1) {
        printf("Error writing questionFile\n");
        goto clean;
    }

    //consume space
    n = recvTCPchar(fdTCP, space);

    n = recvTCPword(fdTCP, &Nanswers, NULL);
    numAnswers = toNonNegative(Nanswers);
    if(numAnswers < 0 || numAnswers > 10) {
        printf("To many answers in this question\n");
        goto clean;
    }

    printf("NUM ANSWERS: %ld\n", numAnswers);
    
    for (int i = 1; i <= numAnswers; i++) {
        //get answerNum

        answerFilePath = strdup(path);
        recvTCPword(fdTCP, &answerN, NULL);
        answerNum = toNonNegative(answerN);

        //get answerID

        recvTCPword(fdTCP, &aUserID, NULL);

        sprintf(tmp, "_%02ld", answerNum);
        answerFilePath = safestrcat(answerFilePath, tmp);
        if((n = writeQuestion(fdTCP, aUserID, answerFilePath)) == -1) {
            printf("Error with image file %d\n", i);
            goto clean;
        }
        free(answerFilePath);
        free(aUserID);
        free(answerN);

        //consume space or /n

        n = recvTCPchar(fdTCP, space);
    }

    clean:
    closedir(topicDirp);
    if(qUserID != NULL) free(qUserID);
    if(res != NULL) free(res);
    if(path != NULL) free(path);
    if(Nanswers != NULL) free(Nanswers);

    return;
}

