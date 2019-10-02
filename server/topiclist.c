#include "commands.h"

char* processTopicList(char** args) {
    char* topicListStatus;
    char topicDatafile[BUFFER_SIZE] = {};
    char topicsInfo[BUFFER_SIZE] = {};
    char topicNameAndUser[BUFFER_SIZE] = {};
    char topicUserID[6] = {};
    char numDirs[4] = {};

    DIR* dirp = opendir(TOPICSDIR);
    struct dirent* dp;
    int dircount = 0, n;
    int errno;

    topicListStatus = (char *)malloc(BUFFER_SIZE * sizeof(char));
    if (!topicListStatus) fatal(ALLOC_ERROR);

    if(args[0] == NULL) {
        strcpy(topicListStatus, "ERR\n");
        closedir(dirp);
        return topicListStatus;
    }

    strncpy(topicListStatus, "LTR ", BUFFER_SIZE);

    if (dirp == NULL) {
        strcat(topicListStatus, "0\n");
        mkdir(TOPICSDIR, 0755);
        return topicListStatus;
    }

    while (dp = readdir(dirp)) {
        if(strcmp(dp->d_name, ".") && strcmp(dp->d_name, "..") && dp->d_type == DT_DIR) {
            dircount++;
            sprintf(topicDatafile, TOPICSDIR"/%s/"DATAFILE, dp->d_name);

            FILE * topicData = fopen(topicDatafile, "r");
            if (topicData == NULL) {
                fatal(FILEOPEN_ERROR);
            }

            n = fread(topicUserID,  1, 5, topicData);
            if(n == 0) fatal(FILEREAD_ERROR);

            sprintf(topicNameAndUser, " %s:%s", dp->d_name, topicUserID);
            strcat(topicsInfo, topicNameAndUser);
        }
    }

    sprintf(numDirs, "%d", dircount);
    strcat(topicListStatus, numDirs);
    strcat(topicListStatus, topicsInfo);
    strcat(topicListStatus, "\n");

    closedir(dirp);
    return topicListStatus;

}
