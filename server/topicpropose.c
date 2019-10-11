#include "commands.h"

char* processTopicPropose(char** args) {
    char* pathName, *topicProposeStatus;
    struct dirent* dirInfo;
    DIR* dirPtr;
    FILE* filePtr;
    int numDirs;
    int bufsz;

    
    topicProposeStatus = (char *)malloc(BUFFER_SIZE * sizeof(char));
    if (!topicProposeStatus) fatal(ALLOC_ERROR);

    // Check number of arguments
    if(arglen(args) != 3) {
        printf("Invalid Protocol!");
        strcpy(topicProposeStatus, "ERR\n");
        return topicProposeStatus;
    }

    stripnewLine(args[2]);

    if (!isValidTopic(args[2])) {
        printf("Invalid topic format!\n");
        strcpy(topicProposeStatus, "ERR\n");
        return topicProposeStatus;
    }

    // TODO: Verify this pls... Very maroscated I believe
    dirPtr = opendir(TOPICSDIR);
    if(!dirPtr && errno == ENOENT) { // if not existent dir, create new one
        if(mkdir(TOPICSDIR, 0755))
            fatal(DIRCREATE_ERROR);
        dirPtr = opendir(TOPICSDIR); // ...and open it
    }
    if(!dirPtr) fatal(DIROPEN_ERROR); // check latest opendir


    // check dir for duplicate names; count topics
    numDirs = 0;
    while((dirInfo = readdir(dirPtr)) != NULL) {
        if(dirInfo->d_name[0] == '.') continue;   // ignore ".", ".." and hidden files
        if(!strcmp(dirInfo->d_name, args[2])) {
            if(closedir(dirPtr) == -1) fatal(DIRCLOSE_ERROR);
            strcpy(topicProposeStatus, "PTR DUP\n");
            return topicProposeStatus;
        }
        numDirs++;
    }

    // check number of existing topics
    if(numDirs >= MAXTOPICS) {
        if(closedir(dirPtr) == -1) fatal(DIRCLOSE_ERROR);
        strcpy(topicProposeStatus, "PTR FUL\n");
        return topicProposeStatus;
    }

    // create new topic
    // size of topics dir + max topic name + space for \0 and /
    // strlen(TOPICSDIR)  + TOPIC_MAXLEN   + 2
    bufsz = strlen(TOPICSDIR) + TOPIC_MAXLEN + 2;
    if(!(pathName = (char*)malloc(sizeof(char) * bufsz)))
        fatal(ALLOC_ERROR);
    memset(pathName, 0, strlen(TOPICSDIR) + TOPIC_MAXLEN + 1);
    sprintf(pathName, TOPICSDIR"/%s", args[2]);
    if(mkdir(pathName, 0700) == -1) {
        if(closedir(dirPtr) == -1) fatal(DIRCLOSE_ERROR);
        strcpy(topicProposeStatus, "PTR NOK\n");
        free(pathName);
        return topicProposeStatus;
    }

    // append data filename to path
    pathName = safestrcat(pathName, "/"DATAFILE);
    filePtr = fopen(pathName, "w");
    fputs(args[1], filePtr); // write userID on file
    if(fclose(filePtr)) fatal(DIRCLOSE_ERROR);
    strcpy(topicProposeStatus, "PTR OK\n");
    if(closedir(dirPtr) == -1) fatal(DIRCLOSE_ERROR);

    free(pathName);
    return topicProposeStatus;
}
