#include "commands.h"

char* processTopicPropose(char** args) {
    char newTopic[11];
    char topicDatafile[16];

    char* topicProposeStatus;
    DIR* dirp;
    struct dirent* dp;
    int dircount = 0;

    
    topicProposeStatus = (char *)malloc(BUFFER_SIZE * sizeof(char));
    if (!topicProposeStatus) fatal(ALLOC_ERROR);

    // Check number of arguments
    if(arglen(args) != 3) {
        strcpy(topicProposeStatus, "ERR\n");
        return topicProposeStatus;
    }

    stripnewLine(args[2]);
    printArgs(args);

    dirp = opendir(TOPICSDIR);
    if(!dirp) fatal(DIROPEN_ERROR);

    // check dir for duplicate names; count topics
    while((dp = readdir(dirp)) != NULL) {
        if(dp->d_name[0] == '.') continue;   // ignore ".", ".." and hidden files
        if(!strcmp(dp->d_name, args[2])) {
            if(closedir(dirp) == -1) fatal(DIRCLOSE_ERROR);
            strcpy(topicProposeStatus, "PTR DUP\n");
            return topicProposeStatus;
        }
        dircount++;
    }

    // check number of existing topics
    if(dircount >= MAXTOPICS) {
        if(closedir(dirp) == -1) fatal(DIRCLOSE_ERROR);
        strcpy(topicProposeStatus, "PTR FUL\n");
        return topicProposeStatus;
    }

    // create new topic
    sprintf(newTopic, TOPICSDIR"/%s", args[2]);
    if(mkdir(newTopic, 0700) == -1) {
        if(closedir(dirp) == -1) fatal(DIRCLOSE_ERROR);
        strcpy(topicProposeStatus, "PTR NOK\n");
    }

    sprintf(topicDatafile, TOPICSDIR"/%s/"DATAFILE, args[2]);
    FILE *topicData = fopen(topicDatafile, "w");
    fputs(args[1], topicData); // write userID on file
    fclose(topicData);
    strcpy(topicProposeStatus, "PTR OK\n");
    if(closedir(dirp) == -1) fatal(DIRCLOSE_ERROR);
    return topicProposeStatus;
}
