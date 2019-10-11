#include "clientcommands.h"

void processAnswerSubmit(char **args, addressInfoSet newAddrInfoSet) {
    int fd, n;

    int numArgs = arglen(args);

    if (!isRegistered()) {
        fprintf(stderr, NOT_REGISTERED_ERROR);
        return;
    }

    /* TODO: handle question selection verification */

    if (!(numArgs == 2 || numArgs == 3)) {
        fprintf(stderr, INVALID_AS_ARGS);
        return;
    }
    stripnewLine(args[numArgs-1]);

    fd = socket(newAddrInfoSet.res_TCP->ai_family,
            newAddrInfoSet.res_TCP->ai_socktype,
            newAddrInfoSet.res_TCP->ai_protocol);
    if (fd == -1) fatal(SOCK_CREATE_ERROR);

    n = connect(fd, newAddrInfoSet.res_TCP->ai_addr,
            newAddrInfoSet.res_TCP->ai_addrlen);
    if (n == -1) fatal(SOCK_CONN_ERROR);

    char *text = args[1];
    char *image = numArgs == 3 ? args[2] : NULL;

    n = sendAnswerSubmit(fd, text, image);
    if (n == -1) return;

    n = recvAnswerSubmit(fd);
    if (n == -1) return;

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
        imageFile = fopen(text, "r");
        if (imageFile == NULL) {
            fprintf(stderr, "Error opening image: %s\n", text);
            return -1;
        }
    }

    size = fileSize(textFile);
    sprintf(buffer, "ANS %d %s %s %ld ",
            userID, selectedTopic, selectedQuestion, size);

    sendTCPstring(fd, buffer, strlen(buffer));
    sendTCPfile(fd, textFile);

    if (image) {
        size = fileSize(imageFile);
        char *ext = image + strlen(image) - 1 - 3;

        /* TODO: remove this */
        printf("Extension: %s\n", ext);

        if (*ext != '.') {
            fprintf(stderr, "Invalid image extension\n");
            return -1;
        }
        ext++;  /* skip '.' */

        sprintf(buffer, " 1 %s %ld ", ext, size);
        sendTCPstring(fd, buffer, strlen(buffer));
        sendTCPfile(fd, imageFile);
    }

    if (image) fclose(imageFile);
    fclose(textFile);

    return 0;
}

int recvAnswerSubmit(int fd) {
    return fd;
}
