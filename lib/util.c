#include "util.h"
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/socket.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>

#define INPUT_SIZE 128
#define FILE_READ_SIZE 512

void fatal(const char* buffer) {
    fprintf(stderr, "%s\n", buffer);
    perror("Error");
    exit(1);
}

// TODO: Remove (debug function)
void printArgs(char** buffer) {
    int i = 0;
    while(buffer[i] != NULL) {
        printf("[%d] -> %s\n", i, buffer[i]);
        i++;
    }
}

void printTopicList(char** topicList) {
    for(int i = 0; topicList[i] != 0; i++){
      printf("%02d - %s\n", i+1, topicList[i]);
    }
}

// safe read from stdin
void readCommand(char** bufPtr, int* bufSize) {
    int i;
    char c;

    // prompt
    write(1, "$ ", 2);

    // read from stdin (char by char) reallocates if necessary
    i = 0;
    while((c = getchar()) != '\n' && c != '\0' && c != EOF) {
        (*bufPtr)[i++] = c;
        if(i == *bufSize) {
            *bufSize += INPUT_SIZE;
            *bufPtr = (char*)realloc(*bufPtr, *bufSize * sizeof(char));
        }
    }

    // check if end of input
    if(c == EOF && i == 0) {
        fatal("End of Input. Exiting");
    }

    // terminate string
    (*bufPtr)[i] = '\0';
}

char **tokenize(char *string) {
    int numArgs = 10;
    int i = 0;

    char **args = (char **)malloc(numArgs * sizeof(char *));
    if (!args) fatal(ALLOC_ERROR);

    args[i] = strtok(string, " ");

    while (args[i] != NULL) {
        i++;
        if (i == numArgs) {
            numArgs *= 2;
            args = realloc(args, numArgs * sizeof(char *));
            if (!args) fatal(ALLOC_ERROR);
        }
        args[i] = strtok(NULL, " ");
    }

    return args;
}

int arglen(char **args) {
    int count = 0;
    while (*(args++) != NULL) count++;

    return count;
}

void resetPtrArray(char** array, int max) {
    int i;
    for(i = 0; i < max && array[i] != NULL; i++) {
        free(array[i]);
        array[i] = 0;
    }
    memset(array, 0, max);
}

/* Concatenate strings for an allocated
 * destination string */
char *safestrcat(char *dest, char *src) {
    int destlen = strlen(dest);
    int srclen = strlen(src);

    char *new = realloc(dest, destlen + srclen + 1);
    strcat(new, src);
    return new;
}


char validPort(char* str) {
    return isPositiveNumber(str) && atoi(str) <= MAX_PORT;
}
// return 0 if string contains non digit ascii
char isPositiveNumber(char* str) {
    int i = 0;
    while(str[i] != '\0') {
        if('9' < str[i] || str[i] < '0') return 0;
        i++;
    }
    return 1;
}

/* Convert string to non negative number
 * Returns -1 on error
 */
long toNonNegative(char *str) {
    errno = 0;
    long n = strtol(str, NULL, 10);

    if (errno != 0 || !isPositiveNumber(str)) {
        return -1;
    }

    return n;
}

/* Accepts socket FD and a \0 terminated string.
 * The function makes sure every byte is sent.
 * Returns 1 on success, 0 on failure
 */
int sendTCPstring(int sockfd, char* buffer, size_t n) {
    int sentBytes, bytesToSend;
    bytesToSend = n;

    while(bytesToSend > 0) {
        sentBytes = send(sockfd, buffer, bytesToSend, MSG_NOSIGNAL);                                                //Ignore SIGPIPE when sending large files, because
                                                                                                                    // we need to read server response when something goes wrong
                                                                                                                    //TODO: Is there a better way
        if(sentBytes == -1){
            return 0;
        }
        bytesToSend -= sentBytes;
        buffer+=sentBytes;
    }
    return 1;
}

int sendTCPfile(int sockfd, FILE* file) {
    char* buffer;
    clearerr(file);
    size_t n;

    long sizesent = 0;



    buffer = (char*) malloc(sizeof(char)*FILE_READ_SIZE);
    if(!buffer) fatal(ALLOC_ERROR);

    while(feof(file) == 0) {
        memset(buffer, 0, FILE_READ_SIZE);
        n = fread(buffer, sizeof(char), FILE_READ_SIZE - 1, file);
        sendTCPstring(sockfd, buffer, n);
        sizesent += n;
    }
    free(buffer);
    return 1;
}

/* Acceps socket FD, a buffer and its size
 * Reads from socket until '\n' read. reallocates buffer if needed
 * Returns number of bytes read
 */
int recvTCPline(int sockfd, char** bufferaddr, int* allocsize) {
    char* ptr;
    char *buffer;
    int alloc = INPUT_SIZE;

    if(allocsize != NULL && *allocsize != 0) {
        alloc = *allocsize;
    }
    buffer = (char*)malloc(alloc * sizeof(char));
    if(!buffer) {
        fatal(ALLOC_ERROR);
    }

    ptr = buffer;
    while(recv(sockfd, ptr, 1, 0) == 1) {
        if(*(ptr++) == '\n')
            break; // terminate string and return

        if(ptr - buffer >= alloc) { // resize buffer
            buffer = (char*)realloc(buffer, 2 * alloc);
            if(!buffer) fatal(ALLOC_ERROR);
            ptr = buffer + alloc;
            alloc *= 2;
        }
    }
    *ptr = '\0';
    *bufferaddr = buffer;
    if (allocsize != NULL) *allocsize = alloc;
    return strlen(buffer);
}

/* Get a word from a tcp socket.
 * Allocate a buffer and store its address in the bufferaddr argument.
 * The initial allocation size is given by the value pointed to by the
 * allocsize argument.
 * Return the length of the received word.
 * The total allocated size, which might not correspond to the length
 * of the word, is stored in the allocsize value-result argument.
 * The buffer should be freed by the caller.
 */
int recvTCPword(int sockfd, char** bufferaddr, int* allocsize) {
    char *ptr;
    char *buffer;
    int alloc = INPUT_SIZE;
    int len;

    if(allocsize != NULL && *allocsize != 0) {
        alloc = *allocsize;
    }

    buffer = (char*)malloc(INPUT_SIZE * sizeof(char));
    if(!buffer) {
        fatal(ALLOC_ERROR);
    }

    len = 0;
    ptr = buffer;
    while(recv(sockfd, ptr, 1, 0) == 1) {
        len++;
        if(*ptr == ' ' || *ptr == '\n')
            break;

        ptr++;
        if((ptr - buffer) == alloc) { // resize buffer
            buffer = (char*)realloc(buffer, 2 * alloc);
            if(!(*buffer)) fatal(ALLOC_ERROR);
            ptr = buffer + alloc;
            alloc *= 2;
        }
    }

    *ptr = '\0';
    *bufferaddr = buffer;
    if (allocsize != NULL) *allocsize = alloc;
    return len;
}

char recvTCPchar(int sockfd, char* charPtr) {
    return recv(sockfd, charPtr, 1, 0) != -1 ? 1 : 0;
}

int recvTCPfile(int sockfd, unsigned long long fileSize, FILE* filefd){
    char* buffer;
    int n;

    long sizesent = 0;

    buffer = (char*) malloc(sizeof(char)*FILE_READ_SIZE);
    if(!buffer) fatal(ALLOC_ERROR);

    while(fileSize > 0) {
        memset(buffer, 0, FILE_READ_SIZE);
        if( (n = recv(sockfd, buffer, MIN(FILE_READ_SIZE - 1, fileSize), 0)) == -1)
            fatal(RECV_TCP_ERROR);

        if(fwrite(buffer, 1 ,n, filefd) == 0)
            fatal(FPUTS_ERROR);

        fileSize -= n;
        sizesent += n;
    }
    //buffer = ptr;

    free(buffer);
    return 1;
}

void stripnewLine(char* str) {
    int i = 0;

    while(str[i] != '\n' && str[i] != '\0') i++;
    str[i] = '\0';
}

int isValidTopic(char* topicName) {
    return validate(topicName, TOPIC_MAXLEN);
}

int isValidQuestion(char* questionName) {
    return validate(questionName, QUESTION_MAXLEN);
}

int validate(char* name, int maxlen) {
    int i = 0;
    while(name[i] != '\0' && i < maxlen) {
        if(!isalnum((int)name[i])) {
            return 0;
        }
        i++;
    }
    return i < maxlen || (i == maxlen && name[i] == '\0');
}

long fileSize(FILE *file) {
    long saved, size;

    saved = ftell(file);
    fseek(file, 0, SEEK_END);
    size = ftell(file);
    fseek(file, saved, SEEK_SET);

    return size;
}

void clearSocket(int fdTCP) {
    char buffer[FILE_READ_SIZE];
    int n;
    int toRead = FILE_READ_SIZE;

    while((n = recv(fdTCP, buffer, toRead, 0)) != 0) {
        if(n == toRead) {
            toRead += toRead;
        }
    }
}
