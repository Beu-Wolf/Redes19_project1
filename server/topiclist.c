#include "commands.h"

char* processTopicList(char** args) {
    char* topicListStatus, *topicDatafile, *topicsInfo, *topicNameAndUser;
    char topicUserID[6] = {};
    char numDirs[4] = {};

    DIR* dirp;
    FILE* topicData;
    struct dirent* dirInfo;
    int dircount;
    int errno;

    topicListStatus = (char *)malloc(BUFFER_SIZE * sizeof(char));
    if (!topicListStatus) fatal(ALLOC_ERROR);

    if(arglen(args) < 1) {
        strncpy(topicListStatus, "ERR\n", 5);
        return topicListStatus;
    }

    strncpy(topicListStatus, "LTR ", BUFFER_SIZE);

    dirp = opendir(TOPICSDIR);
    if (!dirp) {
        strcat(topicListStatus, "0\n");
        mkdir(TOPICSDIR, 0755);
        return topicListStatus;
    }

    topicDatafile = (char*)malloc(BUFFER_SIZE * sizeof(char));
    topicsInfo = (char*)malloc(BUFFER_SIZE * sizeof(char));
    topicNameAndUser = (char*)malloc(BUFFER_SIZE * sizeof(char));
    if(!topicDatafile || !topicsInfo || !topicNameAndUser) fatal(ALLOC_ERROR);

    memset(topicDatafile, 0, BUFFER_SIZE);
    memset(topicsInfo, 0, BUFFER_SIZE);
    memset(topicNameAndUser, 0, BUFFER_SIZE);

    dircount = 0;
    while ((dirInfo = readdir(dirp))) {
        if(dirInfo->d_name[0] == '.' || dirInfo->d_type != DT_DIR) continue;
        sprintf(topicDatafile, TOPICSDIR"/%s/"DATAFILE, dirInfo->d_name);

        topicData = fopen(topicDatafile, "r");
        if (!topicData) fatal(FILEOPEN_ERROR);
        if(!fread(topicUserID,  1, 5, topicData)) fatal(FILEREAD_ERROR);
        fclose(topicData);

        sprintf(topicNameAndUser, " %s:%s", dirInfo->d_name, topicUserID);
        topicsInfo = safestrcat(topicsInfo, topicNameAndUser);

        dircount++;
    }

    sprintf(numDirs, "%d", dircount);
    topicListStatus = safestrcat(topicListStatus, numDirs);
    topicListStatus = safestrcat(topicListStatus, topicsInfo);
    topicListStatus = safestrcat(topicListStatus, "\n");

    closedir(dirp);
    free(topicDatafile);
    free(topicsInfo);
    free(topicNameAndUser);

    return topicListStatus;
}
