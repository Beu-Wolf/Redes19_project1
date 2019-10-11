#include "commands.h"

char* processRegister(char** args) {
    char* registerStatus;

    registerStatus = (char*)malloc(BUFFER_SIZE * sizeof(char));
    if (!registerStatus) fatal(ALLOC_ERROR);

    if(arglen(args) < 2 || !isPositiveNumber(args[1])) {
        strcpy(registerStatus, "ERR\n");
        return registerStatus;
    }

    stripnewLine(args[1]);

    errno = 0;
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
