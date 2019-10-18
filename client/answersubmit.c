#include "clientcommands.h"

void processAnswerSubmit(char **args) {
    int fd;
    int numArgs = arglen(args);
    char* text, *image;

    if (!isRegistered()) {
        fprintf(stderr, NOT_REGISTERED_ERROR);
        return;
    }

    if (!(numArgs == 2 || numArgs == 3)) {
        fprintf(stderr, INVALID_AS_ARGS);
        return;
    }

    if(access(args[1], R_OK)) {
        fprintf(stderr, FILE_NOT_AVAILABLE_ERROR);
        return;
    }

    if(numArgs == 3 && access(args[2], R_OK)) {
        fprintf(stderr, IMAGE_NOT_AVAILABLE_ERROR);
        return;
    }




    stripnewLine(args[numArgs-1]);

    if((fd = socket(tcpInfo->ai_family, tcpInfo->ai_socktype, tcpInfo->ai_protocol)) == -1)
        fatal(SOCK_CREATE_ERROR);

    setSocketTimeout(fd, CLIENT_TIMEOUT);

    if(connect(fd, tcpInfo->ai_addr, tcpInfo->ai_addrlen) == -1)
        fatal(SOCK_CONN_ERROR);

    text = args[1];
    image = numArgs == 3 ? args[2] : NULL;

    if(sendAnswerSubmit(fd, text, image) == -1)
        return;

    if(recvAnswerSubmit(fd) == -1)
        return;

    return;
}

int sendAnswerSubmit(int fd, char *text, char *image) {
    FILE *imageFile, *textFile;
    long size;

    char buffer[BUFFER_SIZE];

    textFile = fopen(text, "r");
    if (textFile == NULL) {
        fprintf(stderr, "Error opening text file: %s\n", text);
        return -1;
    }

    if (image) {
        imageFile = fopen(image, "r");
        if (imageFile == NULL) {
            fprintf(stderr, "Error opening image: %s\n", text);
            return -1;
        }
    }

    size = fileSize(textFile);
    sprintf(buffer, "ANS %s %s %s %ld ",
            userID, selectedTopic, selectedQuestion, size);

    sendTCPstring(fd, buffer, strlen(buffer));
    sendTCPfile(fd, textFile);

    if (image) {
        size = fileSize(imageFile);
        char *ext = image + strlen(image) - 1 - 3;

        if (*ext != '.') {
            fprintf(stderr, "Invalid image extension\n");
            return -1;
        }
        ext++;  /* skip '.' */

        sprintf(buffer, " 1 %s %ld ", ext, size);
        sendTCPstring(fd, buffer, strlen(buffer));
        sendTCPfile(fd, imageFile);
    } else {
        sendTCPstring(fd, " 0", 2);
    }

    sendTCPstring(fd, "\n", 1);
    if (image) fclose(imageFile);
    fclose(textFile);

    return 0;
}

int recvAnswerSubmit(int fd) {
    char *msg;
    char **tokens;
    recvTCPline(fd, &msg, NULL);

    tokens = tokenize(msg);

    if (arglen(tokens) != 2 || strcmp(tokens[0], "ANR")) {
        fprintf(stderr, "Invalid server response.\n");
        return -1;
    }

    stripnewLine(tokens[1]);

    if (!strcmp(tokens[1], "OK")) {
        printf("Answer submitted.\n");
    } else if (!strcmp(tokens[1], "NOK")) {
        printf("Could not perform operation\n");
    } else if (!strcmp(tokens[1], "FUL")) {
        printf("Answer list full.\n");
    } else if (!strcmp(tokens[1], "ERR")) {
        printf("Server error.\n");
    } else {
        fprintf(stderr, "Invalid server response.\n");
        return -1;
    }

    free(tokens);
    free(msg);

    return 0;
}
