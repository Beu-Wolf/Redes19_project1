#ifndef __COMMANDS_H_
#define __COMMANDS_H_

#include <stdio.h>
#include <stdlib.h>
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

#define MAX(A,B) ((A)>= (B) ? (A):(B))

char *processRegister(char **tokenizedMessage);
char *processTopicPropose(char **tokenizedMessage);
char *processTopicList(char **tokenizedMessage);

#endif
