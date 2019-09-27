#include "commands.h"

char* processRegister(char** tokenizedMessage) {
    char* registerStatus = (char *)malloc(BUFFER_SIZE * sizeof(char));
    if (!registerStatus) fatal("Allocation error");

    errno = 0;

    if(tokenizedMessage[1] == NULL) {
        strcpy(registerStatus, "ERR\n");
        return registerStatus;
    }

    int number = strtol(tokenizedMessage[1], NULL, 0);
    if(errno == EINVAL) {
        strcpy(registerStatus, "ERR\n");
        return registerStatus;
    }

    if(number >= 10000 && number <= 99999) {
        strcpy(registerStatus, "RGR OK\n");
    } else {
        strcpy(registerStatus, "RGR NOK\n");
    }

    return registerStatus;

}
