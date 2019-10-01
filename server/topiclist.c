#include "commands.h"

char* processTopicList(char** args) {
    char* topicListStatus = (char *)malloc(BUFFER_SIZE * sizeof(char));
    if (!topicListStatus) exit(1);


    char topicDatafile[BUFFER_SIZE];

    char topicUserID[6];
    char numDirs[4];

    char topicNameAndUser[BUFFER_SIZE];

    char topicsInfo[BUFFER_SIZE];

    memset(topicsInfo, 0, BUFFER_SIZE);

    DIR* dirp = opendir(TOPICSDIR);
    struct dirent* dp;
    int dircount = 0, n;
    int errno;

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
            sprintf(topicDatafile, TOPICSDIR"/%s/%sdata", dp->d_name, dp->d_name);

            FILE * topicData = fopen(topicDatafile, "r");
            if (topicData == NULL) {
                printf("%d\n", errno);
                exit(1);
            }

            n = fread(topicUserID,  1, 5, topicData);
            if(n == 0) exit(1);

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
