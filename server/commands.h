#ifndef __COMMANDS_H_
#define __COMMANDS_H_

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <netdb.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <errno.h>
#include <dirent.h>

#include "../lib/util.h"

#define DEFAULT_PORT "58036"
#define BUFFER_SIZE 1024
#define TOPICSDIR "./topics"
#define DATAFILE  ".data"
#define LOCKFILE ".lock"
#define ANSWERFILE "answer"
#define IMAGEFILE "image"

// FS
void handleSIGCHLD();
void handleSIGINT();
void receiveConnections(char *port);
int setupServerSocket(char *port, int socktype);
void handleTcp(int fd, char* port);
void handleUdp(int fd, char*port);

// UDP
char *processRegister(char **args);
char *processTopicPropose(char **args);
char *processTopicList(char **args);
char *processQuestionList(char **args);

// TCP
void processQuestionSubmit(int fdTCP);
char* getUserId(int fdTCP, int size);
char* getTopic(int fdTCP, int size);
char* getQuestion(int fdTCP, int size);
int getFileSize(int fdTCP, int size);
int getImageFlag(int fdTCP, int size);
char* getImageExtension(int fdTCP, int size);
int getImageFileSize(int fdTCP, int size);

void processQuestionGet(int fdTCP);

void processAnswerSubmit(int fd);

void questionLock(char *topic, char *question);
void questionUnlock(void);
#endif
