#include "clientcommands.h"

void processTopicSelect(char** parsedInput, short argumentShort, char** topicList){

    int wantedNumber;
    errno = 0;
    
    stripnewLine(parsedInput[1]);

    if(argumentShort) {
        printf("Want topic number\n");
        wantedNumber = strtol(parsedInput[1], NULL, 0);

        if(errno != 0) {
            printf("Invalid number\n");
        }
        
        for(int i = 0 ; topicList[i] != 0; i++) {
            if(i == wantedNumber){
                selectedTopic = topicList[i];
                printf("Topic selected: %s\n", selectedTopic);
                return;
            }
        }

        printf("Please select a number from the available topics\n");

    } else {
        printf("Want topic\n");

        for(int i = 0; topicList[i] != 0; i++){
            if(!strcmp(parsedInput[1], topicList[i])) {
                selectedTopic = topicList[i];
                printf("Topic selected: %s\n", selectedTopic);
                return;
            }
        }
        printf("Please select a valid topic name\n");
    }
}