#include "commands.h"

char* processTopicPropose(char** args) {
    char* topicProposeStatus = (char *)malloc(BUFFER_SIZE * sizeof(char));
    if (!topicProposeStatus) exit(1);

    DIR* dirp = opendir(TOPICSDIR);
    struct dirent* dp;
    int dircount = 0, n;

    char newTopic[11];
    char topicDatafile[16];

    if(args[1] == NULL || args[2] == NULL) {
        strcpy(topicProposeStatus, "ERR\n");
        closedir(dirp);
        return topicProposeStatus;
    }

    stripnewLine(args[2]);
    printf("%s", args[2]);

    while(dirp) {
        if((dp = readdir(dirp)) != NULL) {
            if(!strcmp(dp->d_name, args[2])){
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
            sprintf(newTopic, TOPICSDIR"/%s", args[2]);
            n = mkdir(newTopic, 0700);

            if(n == -1) {
                strcpy(topicProposeStatus, "PTR NOK\n");
                closedir(dirp);
                break;
            }


            sprintf(topicDatafile, TOPICSDIR"/%s/data", args[2]);

            FILE *topicData = fopen(topicDatafile, "w");

            fputs(args[1], topicData);

            fclose(topicData);

            strcpy(topicProposeStatus, "PTR OK\n");
            closedir(dirp);
            return topicProposeStatus;
        }
    }
}
