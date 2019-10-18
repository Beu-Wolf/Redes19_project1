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

/*
 *   Prints error messages and exits
 */
void fatal(const char* buffer) {
    fprintf(stderr, "%s\n", buffer);
    perror("Error");
    exit(1);
}

/*
 *   Prints array of strings
 */
void printArgs(char** buffer) {
    int i = 0;
    while(buffer[i] != NULL) {
        printf("[%d] -> %s\n", i, buffer[i]);
        i++;
    }
}

/*
 *   Prints topic List
 */
void printTopicList(char** topicList) {
    for(int i = 0; topicList[i] != 0; i++){
      printf("%02d - %s\n", i+1, topicList[i]);
    }
}


/*
 *   Safe read from stdin
 */
void readCommand(char** bufPtr, int* bufSize) {
    int i;
    char c;

    // prompt
    write(1, "$ ", 2);

    // read char by char and reallocates if necessary
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

/*
 *   Returns NULL terminated array of pointers
 *   Each pointer points to a word of string
 *   (Allocates array. Has to be freed)
 */
char **tokenize(char *string) {
    int numArgs = 10;
    int i = 0;
    char *p;

    char **args = (char **)malloc(numArgs * sizeof(char *));
    if (!args) fatal(ALLOC_ERROR);

    p = string;
    args[i++] = p;

    p = strchr(p, ' ');
    while (p != NULL) {
        *p = '\0';
        p++;
        args[i++] = p;

        if (i == numArgs) {
            numArgs *= 2;
            args = (char **)realloc(args, numArgs * sizeof(char *));
        }
        p = strchr(p, ' ');
    }

    args[i] = NULL;

    return args;
}


/*
 *   Return number of args
 */
int arglen(char **args) {
    int count = 0;
    while (*(args++) != NULL) count++;

    return count;
}

/*
 *   Free and clear array of pointers
 */
void resetPtrArray(char** array, int max) {
    int i;
    for(i = 0; i < max && array[i] != NULL; i++) {
        free(array[i]);
        array[i] = 0;
    }
    memset(array, 0, max);
}

/*
 *   Concatenate strings for an allocated
 *   destination string
 */
char *safestrcat(char *dest, char *src) {
    int destlen = strlen(dest);
    int srclen = strlen(src);

    char *new = realloc(dest, destlen + srclen + 1);
    strcat(new, src);
    return new;
}


/*
 *   Return if str represents a valid port number
 */
char validPort(char* str) {
    return isPositiveNumber(str) && atoi(str) <= MAX_PORT;
}

/*
 *   Return if str represents a non-negative number
 *   (if it only contains digits)
 */
char isPositiveNumber(char* str) {
    int i = 0;
    while(str[i] != '\0') {
        if('9' < str[i] || str[i] < '0') return 0;
        i++;
    }
    return 1;
}

/*
 *   Convert string to non negative number
 *   Returns -1 on error
 */
long toNonNegative(char *str) {
    errno = 0;
    long n = strtol(str, NULL, 10);

    if (errno != 0 || !isPositiveNumber(str)) {
        return -1;
    }

    return n;
}

/*
 *   Accepts socket FD and a \0 terminated string.
 *   The function makes sure every byte is sent.
 *   Returns 1 on success, 0 on failure
 */
int sendTCPstring(int sockfd, char* buffer, size_t n) {
    int sentBytes, bytesToSend;
    bytesToSend = n;

    while(bytesToSend > 0) {
        sentBytes = send(sockfd, buffer, bytesToSend, MSG_NOSIGNAL); // Ignore SIGPIPE
        if(sentBytes == -1){
            return 0;
        }
        bytesToSend -= sentBytes;
        buffer+=sentBytes;
    }
    return 1;
}


/*
 *   Accepts socket FD and a file pointer.
 *   The function makes sure every byte is sent.
 *   Returns 1 on success, 0 on failure
 */
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

/*
 *   Acceps socket FD, a buffer and its size
 *   Reads from socket until '\n' read. reallocates buffer if needed
 *   Returns number of bytes read
 */
int recvTCPline(int sockfd, char** bufferaddr, int* allocsize) {
    char* ptr;
    char *buffer;
    int alloc = INPUT_SIZE;
    int ret;

    if(allocsize != NULL && *allocsize != 0) {
        alloc = *allocsize;
    }
    buffer = (char*)malloc(alloc * sizeof(char));
    if(!buffer) {
        fatal(ALLOC_ERROR);
    }

    ptr = buffer;
    while((ret = recv(sockfd, ptr, 1, 0)) == 1) {
        if(*(ptr++) == '\n')
            break; // terminate string and return

        if(ptr - buffer >= alloc) { // resize buffer
            buffer = (char*)realloc(buffer, 2 * alloc);
            if(!buffer) fatal(ALLOC_ERROR);
            ptr = buffer + alloc;
            alloc *= 2;
        }
    }

    if (ret == -1) {
        if (errno == EAGAIN || errno == EWOULDBLOCK) {
            timedOut();
        }
    }

    *ptr = '\0';
    *bufferaddr = buffer;
    if (allocsize != NULL) *allocsize = alloc;
    return strlen(buffer);
}

/*
 *   Get a word from a tcp socket.
 *   Allocate a buffer and store its address in the bufferaddr argument.
 *   The initial allocation size is given by the value pointed to by the
 *   allocsize argument.
 *   Return the length of the received word.
 *   The total allocated size, which might not correspond to the length
 *   of the word, is stored in the allocsize value-result argument.
 *   The buffer should be freed by the caller.
 */
int recvTCPword(int sockfd, char** bufferaddr, int* allocsize) {
    char *ptr;
    char *buffer;
    int alloc = INPUT_SIZE;
    int len;
    int ret;

    if(allocsize != NULL && *allocsize != 0) {
        alloc = *allocsize;
    }

    buffer = (char*)malloc(INPUT_SIZE * sizeof(char));
    if(!buffer) {
        fatal(ALLOC_ERROR);
    }

    len = 0;
    ptr = buffer;
    while((ret = recv(sockfd, ptr, 1, 0)) == 1) {
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

    if (ret == -1) {
        if (errno == EAGAIN || errno == EWOULDBLOCK) {
            timedOut();
        }
    }

    *ptr = '\0';
    *bufferaddr = buffer;
    if (allocsize != NULL) *allocsize = alloc;
    return len;
}

/*
 *   Reads a char from tcp into *p.
 *   Returns 1 on success, 0 on failure
 */
int recvTCPchar(int fd, char *p) {
    int ret;
    ret = recv(fd, p, 1, 0);
    if (ret == -1) {
        if (errno == EAGAIN || errno == EWOULDBLOCK) {
            timedOut();
        }
        return 0;
    }

    return 1;
}

/*
 *   Receives a file from TCP
 *   (receives fileSize bytes only)
 *   Returns 1 on success, 0 on failure
 */
int recvTCPfile(int sockfd, unsigned long long fileSize, FILE* filefd){
    char* buffer;
    int n;

    long sizesent = 0;

    buffer = (char*) malloc(sizeof(char)*FILE_READ_SIZE);
    if(!buffer) fatal(ALLOC_ERROR);
    while(fileSize > 0) {
        memset(buffer, 0, FILE_READ_SIZE);
        if( (n = recv(sockfd, buffer, MIN(FILE_READ_SIZE - 1, fileSize), 0)) == -1) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                timedOut();
            }
            fatal(RECV_TCP_ERROR);
        }

        if(fwrite(buffer, 1 ,n, filefd) == 0)
            fatal(FPUTS_ERROR);

        fileSize -= n;
        sizesent += n;
    }
    //buffer = ptr;

    free(buffer);
    return 1;
}


/*
 *   Terminates string on \n
 */
void stripnewLine(char* str) {
    int i = 0;

    while(str[i] != '\n' && str[i] != '\0') i++;
    str[i] = '\0';
}

/*
 *   Returns if valid topic name
 */
int isValidTopic(char* topicName) {
    return validate(topicName, TOPIC_MAXLEN);
}

/*
 *   Returns if valid question name
 */
int isValidQuestion(char* questionName) {
    return validate(questionName, QUESTION_MAXLEN);
}

/*
 *   Returns if alphanumeric string
 *   with no more than maxlen characters
 */
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

/*
 *   return filesize
 */
long fileSize(FILE *file) {
    long saved, size;

    saved = ftell(file);
    fseek(file, 0, SEEK_END);
    size = ftell(file);
    fseek(file, saved, SEEK_SET);

    return size;
}

/*
 *   Clear socket contents
 */
void clearSocket(int fdTCP) {
    char c[FILE_READ_SIZE];
    int bRead;
    while((bRead = read(fdTCP, &c, FILE_READ_SIZE)) != 0 || bRead == -1) {
        if(bRead == -1) fatal("Clearing socket");
    }
}

/*
 *   set socket timeout
 */
void setSocketTimeout(int fd, int seconds) {
    struct timeval t;
    t.tv_sec = seconds;
    t.tv_usec = 0;

    setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, (const char*)&t, sizeof(t));
}

/*
 * timeout callback
 */
void timedOut() {
    fatal("Timed out waiting for response.");
}
