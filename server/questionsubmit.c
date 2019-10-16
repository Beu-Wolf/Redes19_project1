#include "commands.h"



char* getUserId(int fdTCP, int size) {
    char* userId; 

    recvTCPword(fdTCP, &userId, &size); 
    printf("UserID: |%s|\n", userId);
    return userId;
}

char* getTopic(int fdTCP, int size) {
    char* topic;

    if(recvTCPword(fdTCP, &topic, &size) > 11) {
        free(topic);
        return NULL;
    }
    printf("Topic: |%s|\n", topic);

    return topic;
}

char* getQuestion(int fdTCP, int size) {
    char* question;

    if(recvTCPword(fdTCP, &question, &size) > 11) {
        free(question);
        return NULL;
    }
    printf("Question: |%s|\n", question);

    return question;
}

int getFileSize(int fdTCP, int size) {
    char* fileSizeStr;
    int fileSize;

    if(recvTCPword(fdTCP, &fileSizeStr, &size) > 11) {
        free(fileSizeStr);
        return -1;
    }

    printf("File Size: |%s|\n", fileSizeStr);

    if(!isPositiveNumber(fileSizeStr)){
        free(fileSizeStr);
        return -1;
    }

    fileSize = strtol(fileSizeStr, NULL, 0);

    free(fileSizeStr);
    return fileSize;
}

int getImageFlag(int fdTCP, int size) {
    char* hasImageStr;
    int hasImage;

    char space[2];

    //clean space between text file and image flag
    recv(fdTCP, space, 1, 0);

    recvTCPword(fdTCP, &hasImageStr, &size);
    printf("Has Image flag |%s|\n", hasImageStr);

    if(!isPositiveNumber(hasImageStr)) {
        free(hasImageStr);
        return -1;
    }

    hasImage = strtol(hasImageStr, NULL, 0);

    free(hasImageStr);
    return hasImage;
}

char* getImageExtension(int fdTCP, int size) {
    char* imgExt;

    if(recvTCPword(fdTCP, &imgExt, &size) != 4) {
        printf("Invalid Extension\n");
        free(imgExt);
        return NULL;
    }

    printf("Image Ext: |%s|\n", imgExt);
    return imgExt;
}

int getImageFileSize(int fdTCP, int size) {
    char* imageFileStr;
    int imageFileSize;

    if(recvTCPword(fdTCP, &imageFileStr, &size) > 11) {
        free(imageFileStr);
        return -1;
    }
    printf("File Size: |%s|\n", imageFileStr);

    if(!isPositiveNumber(imageFileStr)){
        free(imageFileStr);
        return -1;
    }

    imageFileSize = strtol(imageFileStr, NULL, 0);

    free(imageFileStr);
    return imageFileSize;
}

void processQuestionSubmit(int fdTCP) {
    char* userId, *topic, *question;
    int size, numQuestions,  n, hasImage;
    long fileSize;
    char response[9];


    size = 6;                   // size of userID plus one
    userId = getUserId(fdTCP, size);


    // Read topic
    size = 11; // size of userID plus one
    topic = getTopic(fdTCP, size);
    if(!topic) {
        printf("Error getting topic\n");
        return;
    }


    char path[BUFFER_SIZE];
    strncpy(path, TOPICSDIR"/", strlen(TOPICSDIR) + 3);
    strcat(path, topic);                    //Topics folder

    DIR* topicDirp = opendir(path);         //Open topics folder
    if(!topicDirp) {
        strncpy(response, "ERR\n", 5);                  //send error message
        sendTCPstring(fdTCP, response, strlen(response));
        return;
    }

    size = 11;                                      //size of question +1
    question = getQuestion(fdTCP, size);

    struct dirent *questionEnt;


    //Question verifications (Duplicates, Full, etc)
    numQuestions = 0;
    while((questionEnt = readdir(topicDirp))) {
        char *currQuestion = questionEnt->d_name;

        if (!strcmp(currQuestion, ".")
                || !strcmp(currQuestion, "..")
                || !strcmp(currQuestion, DATAFILE)) {
            continue;
        }

        if(!strcmp(currQuestion, question)) {
            strncpy(response, "QUR DUP\n", 9); 
            printf("Duplicado\n");                 //send duplicate message
            sendTCPstring(fdTCP, response, strlen(response));
            closedir(topicDirp);
            return;
        }

        numQuestions++;

        if(numQuestions == 99) {
            strncpy(response, "QUR FUL\n", 9);     
            printf("FULL\n");             //send full message
            sendTCPstring(fdTCP, response, strlen(response));
            closedir(topicDirp);
            return;
        }
    }

    closedir(topicDirp);

    //create folder

    strcat(path, "/");
    strcat(path, question);

    n = mkdir(path, 0700);

    if(n == -1) {
            strncpy(response, "QUR NOK\n", 9);                  //send not ok message
            sendTCPstring(fdTCP, response, strlen(response));
            return;
    }

    //create dataFile
    char* questionFolder = (char*) malloc(sizeof(char)*BUFFER_SIZE);
    strncpy(questionFolder, path, strlen(path) + 2);
    questionFolder = safestrcat(questionFolder, "/");
    questionFolder = safestrcat(questionFolder, DATAFILE);

                        

    FILE *questionDatafile = fopen(questionFolder, "a");
    fputs(userId, questionDatafile);



    //Receive filesize
    size = 11; // size of fileSize plus one
    fileSize = getFileSize(fdTCP, size);

    if(fileSize == -1) {
        strncpy(response, "QUR NOK\n", 9);                  //send not ok message
        sendTCPstring(fdTCP, response, strlen(response));
        return;
    }


    //create question txt

    char* questionFile = (char*) malloc(sizeof(char)*BUFFER_SIZE);
    strncpy(questionFile, path, strlen(path) + 2);
    questionFile = safestrcat(questionFile, "/");
    questionFile = safestrcat(questionFile, question);
    questionFile = safestrcat(questionFile, ".txt");

    FILE* questionFilePtr = fopen(questionFile, "w");

    if(!questionFilePtr) {
        strncpy(response, "QUR NOK\n", 9);                  //send not ok message
        sendTCPstring(fdTCP, response, strlen(response));
        return;
    }

    //receive File and write
    recvTCPfile(fdTCP, fileSize, questionFilePtr);

    //check or existance of image
    size = 2;                                           //flag size + 1;
    hasImage = getImageFlag(fdTCP, size);

    if(hasImage == -1) {
        strncpy(response, "QUR NOK\n", 9);                  //send not ok message
        fclose(questionDatafile);
        sendTCPstring(fdTCP, response, strlen(response));
        return;
    } else if(hasImage == 1) {
        //read image extension
        size = 4;                                       //size of imageExtension + 1 
        char* imgExt = getImageExtension(fdTCP, size);
        if(!imgExt) {
            strncpy(response, "QUR NOK\n", 9);                  //send not ok message
            sendTCPstring(fdTCP, response, strlen(response));
            fclose(questionDatafile);
            return;
        }

        fprintf(questionDatafile, " %s", imgExt);

        
        fclose(questionDatafile);


        size = 11; // size of fileSize plus one
        fileSize = getImageFileSize(fdTCP, size);

        if(fileSize == -1) {
            strncpy(response, "QUR NOK\n", 9);                  //send not ok message
            sendTCPstring(fdTCP, response, strlen(response));
            return;
        }


        //create Image file
        char* imageFile = (char*) malloc(sizeof(char)*BUFFER_SIZE);
        strncpy(imageFile, path, strlen(path) + 3);
        imageFile = safestrcat(imageFile, "/");
        imageFile = safestrcat(imageFile, question);
        imageFile = safestrcat(imageFile, ".");
        imageFile = safestrcat(imageFile, imgExt);

        FILE* imageFilePtr = fopen(imageFile, "w");

        if(!imageFilePtr) {
            strncpy(response, "QUR NOK\n", 9);                  //send not ok message
            sendTCPstring(fdTCP, response, strlen(response));
            return;
        }

        recvTCPfile(fdTCP, fileSize, imageFilePtr);
        free(imageFile);
        fclose(imageFilePtr);

    } else {
        fclose(questionDatafile);
    }


    
    



    strncpy(response, "QUR OK\n", 8);                  //send not ok message
    sendTCPstring(fdTCP, response, strlen(response));
    fclose(questionFilePtr);

    
    
    free(questionFolder);
    free(questionFile);
    free(userId);
    return;
}
