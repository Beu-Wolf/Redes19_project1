#include "commands.h"

char* processRegister(char** args) {
    char* registerStatus = (char *)malloc(BUFFER_SIZE * sizeof(char));
    if (!registerStatus) fatal(ALLOC_ERROR);

    errno = 0;

    if(args[1] == NULL) {
        strcpy(registerStatus, "ERR\n");
        return registerStatus;
    }

    int number = strtol(args[1], NULL, 0);
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
