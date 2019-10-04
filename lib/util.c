#include "util.h"
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/socket.h>
#include <string.h>

#define INPUT_SIZE 128
#define FILE_READ_SIZE 512

void fatal(const char* buffer) {
    fprintf(stderr, "%s\n", buffer);
    perror("Error");
    exit(1);
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

/* Concatenate strings for an allocated
 * destination string */
char *safestrcat(char *dest, char *src) {
    int destlen = strlen(dest);
    int srclen = strlen(src);

    char *new = realloc(dest, destlen + srclen + 1);
    strcat(new, src);
    return new;
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

/* Accepts socket FD and a \0 terminated string.
 * The function makes sure every byte is sent.
 * Returns 1 on success, 0 on failure
 */
int sendTCPstring(int sockfd, char* buffer) {
    int sentBytes, bytesToSend;
    bytesToSend = strlen(buffer);
    while(bytesToSend > 0) {
        sentBytes = send(sockfd, buffer, bytesToSend, 0);
        if(sentBytes == -1)
            return 0;

        bytesToSend -= sentBytes;
        buffer+=sentBytes;
    }
    return 1;
}

int sendTCPfile(int sockfd, FILE* file) {
    char* buffer;

    buffer = (char*) malloc(sizeof(char)*FILE_READ_SIZE); 
    if(!buffer) fatal(ALLOC_ERROR);

    while(feof(file) == 0) {
        memset(buffer, 0, FILE_READ_SIZE);
        fread(buffer, sizeof(char), FILE_READ_SIZE - 1, file);
        printf("%s\n", buffer);
        sendTCPstring(sockfd, buffer);
    }

    free(buffer);
    return 1;
}

/* Acceps socket FD, a buffer and its size
 * Reads from socket until '\n' read. reallocates buffer if needed
 * Returns number of bytes read
 */
int recvTCPline(int sockfd, char** buffer, int* size) {
    char* ptr;

    // TODO: Is this really necessary???
    // allocate buffer if needed
    if(*buffer == NULL || *size == 0) {
        (*buffer) = (char*)malloc(INPUT_SIZE * sizeof(char));
        if(!(*buffer)) {
            fatal(ALLOC_ERROR);
        }
        *size = INPUT_SIZE;
    }

    ptr = *buffer;
    while(recv(sockfd, ptr, 1, 0) == 1) {
        if(*(ptr++) == '\n')
            break; // terminate string and return

        if(ptr - (*buffer) >= *size) { // resize buffer
            *buffer = (char*)realloc(*buffer, 2 * *size);
            if(!(*buffer)) fatal(ALLOC_ERROR);
            ptr = *buffer + *size;
            (*size) *= 2;
        }
    }
    *ptr = '\0';
    return strlen(*buffer);
}


int recvTCPword(int sockfd, char** buffer, int* size) {
    char* ptr;
    int len;

    // TODO: Is this really necessary???
    // allocate buffer if needed
    if(*buffer == NULL || *size == 0) {
        (*buffer) = (char*)malloc(INPUT_SIZE * sizeof(char));
        if(!(*buffer)) {
            fatal(ALLOC_ERROR);
        }
        *size = INPUT_SIZE;
    }

    len = 0;
    ptr = *buffer;
    while(recv(sockfd, ptr, 1, 0) == 1) {
        len++;
        if(*ptr == ' ' || *ptr == '\n')
            break; // terminate string and return

        ptr++;
        if((ptr - (*buffer)) == *size) { // resize buffer
            *buffer = (char*)realloc(*buffer, 2 * *size);
            if(!(*buffer)) fatal(ALLOC_ERROR);
            ptr = *buffer + *size;
            (*size) *= 2;
        }
    }
    *ptr = '\0';
    return len;
}

int recvTCPfile(int sockfd, unsigned long long fileSize, FILE* filefd){
    char* buffer, *ptr;
    int n, bytesToRead;
    int currSize = fileSize; //To keep track of when to stop reading

    bytesToRead = fileSize;

    buffer = (char*) malloc(sizeof(char)*FILE_READ_SIZE); 
    if(!buffer) fatal(ALLOC_ERROR);

    

    ptr = buffer;
    while(currSize > 0) {

        if(currSize > FILE_READ_SIZE) {
            bytesToRead = FILE_READ_SIZE;
            buffer = ptr;
        } else {
            bytesToRead = currSize;
        }

        n = recv(sockfd, buffer, bytesToRead, 0);
        fputs(buffer, filefd);
        printf("Writing:%s\n", buffer);
        
        currSize -= n;
        buffer += n;
    }
    buffer = ptr;

    free(buffer);
    return 1;
}

void stripnewLine(char* str) {
    int i = 0;

    while(str[i] != '\n' && str[i] != '\0') i++;
    str[i] = '\0';
}
