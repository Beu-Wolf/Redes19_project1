#include "clientcommands.h"

void processTopicSelect(char** parsedInput, char** topicList){
    int wantedNumber = -1;
    char abbrev;

    if(!isRegistered()) {
      fprintf(stderr, NOT_REGISTERED_ERROR);
      return;
    }

    if(arglen(parsedInput) != 2) {
      fprintf(stderr, INVALID_TS_ARGS);
      return;
    }

    // get topic number
    abbrev = strlen(parsedInput[0]) == 2;
    if(abbrev) {
        errno = 0;
        wantedNumber = strtol(parsedInput[1], NULL, 10);
        if(errno != 0 || !isPositiveNumber(parsedInput[1])) {
            printf("Invalid number\n");
            return;
        }
    }

    // find topic
    stripnewLine(parsedInput[1]);
    for(int i = 0; topicList[i] != 0; i++) {
      if((abbrev && i == wantedNumber) || (!abbrev && !strcmp(parsedInput[1], topicList[i]))) {
        selectedTopic = topicList[i];
        printf("Selected topic: %s\n", selectedTopic);
        return;
      }
    }
    printf("Please select a valid topic\n");
}
