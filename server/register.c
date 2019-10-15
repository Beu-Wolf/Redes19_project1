#include "commands.h"

char* processRegister(char** args) {
    char* registerStatus;

    registerStatus = (char*)malloc(BUFFER_SIZE * sizeof(char));
    if (!registerStatus) fatal(ALLOC_ERROR);

    if(arglen(args) != 2) {
        strncpy(registerStatus, "ERR\n", 5);
        return registerStatus;
    }

    stripnewLine(args[1]);

    if(!isPositiveNumber(args[1])) {
        strncpy(registerStatus, "ERR\n", 5);
        return registerStatus;
    }


    errno = 0;
    int number = strtol(args[1], NULL, 0);
    if(errno == EINVAL) {
        strncpy(registerStatus, "ERR\n", 5);
        return registerStatus;
    }

    if(number >= 10000 && number <= 99999) {
        strncpy(registerStatus, "RGR OK\n", 8);
    } else {
        strncpy(registerStatus, "RGR NOK\n", 9);
    }

    return registerStatus;

}
