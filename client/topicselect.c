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

    if(topicList[0] == NULL) {
        printf("No topics to show. Use topic_list (tl) to get a list of available topics.\n");
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
    for(int i = 1; topicList[i-1] != 0; i++) {  
      if((abbrev && i == wantedNumber) || (!abbrev && !strcmp(parsedInput[1], topicList[i-1]))) {
        selectedTopic = topicList[i-1];
        printf("Selected topic: %s\n", selectedTopic);
        return;
      }
    }
    printTopicList(topicList);
    printf("Please select a valid topic\n");
}
