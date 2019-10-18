#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>
#include <limits.h>
#include <stdio.h>
#include <arpa/inet.h>
#include <errno.h>
#include "../lib/util.h"
#include "clientcommands.h"

// funciton declaration
static void displayUsage();

void readArguments();
void init();
void interact();
void cleanup();

// argument-flag variables
char* server; // always alloced
char* port; // never alloced

// socketinfo
int fdUDP;
char** topicList;
char** questionList;


char isRegistered() {
  return userID != NULL;
}

int main(int argc, char** argv) {
    readArguments(argc, argv);
    init();
    interact();
    cleanup();
    return 0;
}

void readArguments(int argc, char** argv) {
    int opt, opterr, i;

    opterr = 0;
    while((opt = getopt(argc, argv, "n:p:h?")) != -1) {
        switch(opt) {
            case 'n':
                server = strdup(optarg);
                break;
            case 'p':
                port = optarg;
                break;
            case '?':
            case 'h':
            default:
                opterr++;
                break;
        }
    }

    for (i = optind; i < argc-1; i++) {
        fprintf(stderr, "Non-option argument: %s\n", argv[i]);
        opterr++;
    }

    if(opterr) displayUsage();

}

static void displayUsage (){
    puts("Usage: user [options]");
    puts("Options:                                (defaults)");
    puts("    n          server identification    (localhost)");
    puts("    p          [p]ort to connect to     (58011)");
    exit(1);
}

void init() {
    int err;
    struct addrinfo hints;

    userID = NULL;
    selectedTopic = NULL;
    selectedQuestion = NULL;

    // question and topic buffers
    topicList = (char**)malloc(sizeof(char*) * MAXTOPICS + 1);
    questionList = (char**)malloc(sizeof(char*) * MAXQUESTIONS + 1);
    if(!topicList || !questionList)
        fatal(ALLOC_ERROR);

    memset(topicList, 0, sizeof(char*) * MAXTOPICS + 1);
    memset(questionList, 0, sizeof(char*) * MAXQUESTIONS + 1);


    // set default server
    if(!server) { 
        if(!(server = (char*)malloc((sizeof(char) * HOST_NAME_MAX) + 1)))
            fatal(ALLOC_ERROR);

        if(gethostname(server, HOST_NAME_MAX) == -1)
            fatal(GETHOSTNAME_ERROR);
    }
    
    // set default port
    if(!port) port = DEFAULT_PORT;

     if(!validPort(port)) {
        fprintf(stderr, "Invalid port number\n");
        exit(1);
    }


    // set TCP addrinfo
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    hints.ai_flags = AI_NUMERICSERV;
    if((err = getaddrinfo(server, port, &hints, &tcpInfo)))
        fatal(gai_strerror(err));
    // set UDP addrinfo
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_protocol = IPPROTO_UDP;
    hints.ai_flags = AI_NUMERICSERV;
    if((err = getaddrinfo(server, port, &hints, &udpInfo)))
        fatal(gai_strerror(err));

    // create UDP socket
    if((fdUDP = socket(udpInfo->ai_family, udpInfo->ai_socktype, udpInfo->ai_protocol)) == -1)
        fatal(SOCK_CREATE_ERROR);
}

void interact() {
    int inpSize;
    char* input, *cmd;
    char** parsedInput;

    inpSize = INPUT_SIZE;
    input = (char*)malloc(inpSize * sizeof(char));
    if(input == NULL) fatal(ALLOC_ERROR);
    while(1) {
        readCommand(&input, &inpSize);
        parsedInput = tokenize(input);
        cmd = parsedInput[0];

        if(!cmd) {
            free(parsedInput);
            continue;
        }

        if(!strcmp(cmd, "reg") || !strcmp(cmd, "register")) {
            processRegister(fdUDP, parsedInput);
            
        } else if(!strcmp(cmd, "tl") || !strcmp(cmd, "topic_list")) {
            processTopicList(fdUDP, parsedInput, topicList);

        } else if(!strcmp(cmd, "ts") || !strcmp(cmd, "topic_select")) {
            processTopicSelect(parsedInput, topicList);

        } else if(!strcmp(cmd, "tp") || !strcmp(cmd, "topic_propose")) {
            processTopicPropose(fdUDP, parsedInput, topicList);

        } else if(!strcmp(cmd, "ql") || !strcmp(cmd, "question_list")) {
            processQuestionList(fdUDP, parsedInput, questionList);

        } else if(!strcmp(cmd, "qg") || !strcmp(cmd, "question_get")) {
            processQuestionGet(parsedInput, questionList);

        } else if(!strcmp(cmd, "qs") || !strcmp(cmd, "question_submit")) {
            processQuestionSubmit(parsedInput, questionList);

        } else if(!strcmp(cmd, "as") || !strcmp(cmd, "answer_submit")) {
            processAnswerSubmit(parsedInput);

        } else if(!strcmp(cmd, "exit")) {
            break;

        } else {
            printf("command not valid. Please try again\n");
        }
    
        free(parsedInput);
    }

    free(parsedInput);
    free(input);
}

void cleanup() {
    freeaddrinfo(tcpInfo);
    freeaddrinfo(udpInfo);
    free(server);

    if(userID) free(userID);

    resetPtrArray(topicList, MAXTOPICS + 1);
    free(topicList);
    resetPtrArray(questionList, MAXQUESTIONS+ 1);
    free(questionList);
}

