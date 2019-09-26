#include "commands.h"

char* processTopicPropose(char** tokenizedMessage) {
    char* topicProposeStatus = (char *)malloc(BUFFER_SIZE * sizeof(char));
    if (!topicProposeStatus) exit(1);

    DIR* dirp = opendir("./topics");
    struct dirent* dp;
    int dircount = 0, n;

    char newTopic[11];
    char topicDatafile[16];

    if(tokenizedMessage[1] == NULL || tokenizedMessage[2] == NULL) {
        strcpy(topicProposeStatus, "ERR\n");
        closedir(dirp);
        return topicProposeStatus;
    }

    stripnewLine(tokenizedMessage[2]);
    printf("%s", tokenizedMessage[2]);
    
    while(dirp) {
        if((dp = readdir(dirp)) != NULL) {
            if(!strcmp(dp->d_name, tokenizedMessage[2])){
                closedir(dirp);
                strcpy(topicProposeStatus, "PTR DUP\n");
                break;
            } else {
                dircount++;
            }

            if(dircount == 50) {
                strcpy(topicProposeStatus, "PTR FUL\n");
                closedir(dirp);
                break;
            }
        } else {
            sprintf(newTopic, "./topics/%s", tokenizedMessage[2]);
            n = mkdir(newTopic, 0700);

            if(n == -1) {
                strcpy(topicProposeStatus, "PTR NOK\n");
                closedir(dirp);
                break;
            }


            sprintf(topicDatafile, "topics/%s/%sdata", tokenizedMessage[2], tokenizedMessage[2]);

            FILE *topicData = fopen(topicDatafile, "w");

            fputs(tokenizedMessage[1], topicData);

            fclose(topicData);

            strcpy(topicProposeStatus, "PTR OK\n");
            closedir(dirp);
            return topicProposeStatus;
        }
    }
}
