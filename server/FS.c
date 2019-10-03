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
#include "commands.h"

#define DEFAULT_PORT "58036"
#define BUFFER_SIZE 1024

#define MAX(A,B) ((A)>= (B) ? (A):(B))

void receiveConnections(char *port);
int setupServerSocket(char *port, int socktype);
void handleTcp(int fd, char* port);
void handleUdp(int fd, char*port);

int main(int argc, char *argv[]) {
    char *port = DEFAULT_PORT;

    if (argc > 1) {
        short err = 0;
        char opt;
        int length;

        opt = getopt(argc, argv, "p:");
        if (opt == 'p') {
            port = optarg;
            length = strlen(port);

            for (int i = 0; i < length; i++) {
                if (!isdigit(port[i])) {
                    err = 1;
                    break;
                }
            }
        } else if (opt == '?' || opt == ':') {
            return 0;
        } else {
            err = 1;
        }

        if (err) {
            fprintf(stderr, "Invalid port number: %s\n", port);
            exit(1);
        }
    }

    // TODO: remove
    printf("Port number: %s\n", port);

    receiveConnections(port);
}

void receiveConnections(char *port) {
    int udpSocket, tcpSocket;

    int newfd;

    int maxfd, counter;
    fd_set rfds;

    udpSocket = setupServerSocket(port, SOCK_DGRAM);
    tcpSocket = setupServerSocket(port, SOCK_STREAM);

    maxfd = MAX(udpSocket, tcpSocket);

    while (1) {
        FD_ZERO(&rfds);
        FD_SET(udpSocket, &rfds);
        FD_SET(tcpSocket, &rfds);

        counter = select(maxfd+1, &rfds, NULL, NULL, NULL);
        if (counter <= 0) fatal(SELECT_ERROR);

        if (FD_ISSET(udpSocket, &rfds)) {
            handleUdp(udpSocket, port);
        }

        if (FD_ISSET(tcpSocket, &rfds)) {
            newfd = accept(tcpSocket, NULL, NULL);
            if (newfd == -1) fatal(SOCK_ACPT_ERROR);

            handleTcp(newfd, port);
        }
    }
}

int setupServerSocket(char *port, int socktype) {
    int fd;
    int n;
    struct addrinfo hints, *res;

    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_INET;
    hints.ai_socktype = socktype;
    hints.ai_flags = AI_PASSIVE | AI_NUMERICSERV;

    n = getaddrinfo(NULL, port, &hints, &res);
    if (n != 0) fatal(GETADDRINFO_ERROR);

    fd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if (fd == -1) fatal(SOCK_CREATE_ERROR);

    n = bind(fd, res->ai_addr, res->ai_addrlen);
    if (n == -1) fatal(SOCK_BIND_ERROR);

    if (socktype == SOCK_STREAM) {
        listen(fd, 5);
    }

    return fd;
}

void handleTcp(int fd, char* port) {
    int pid, ret;
    char buffer[BUFFER_SIZE];

    ret = fork();
    if(ret == -1)
        fatal(FORK_ERROR);

    if (ret != 0) {
        // parent process
        close(fd);
        return;
    }

    // child process
    pid = getpid();
    printf("Forked. PID = %d\n", pid);
    while (1) {
        memset(buffer, 0, BUFFER_SIZE);
        if (read(fd, buffer, BUFFER_SIZE) == 0) // finished reading
            exit(0);

        printf("[TCP][%d] %s\n", pid, buffer);
    }
}

void handleUdp(int fd, char* port) {
    char buffer[BUFFER_SIZE];
    char messageSender[INET_ADDRSTRLEN];

    char* messageToSend;

    char** args;
    struct sockaddr_in addr;
    socklen_t addrlen = sizeof addr;

    memset(&addr, 0, sizeof(addr));


    int n;

    memset(buffer, 0, BUFFER_SIZE);
    recvfrom(fd, buffer, BUFFER_SIZE, 0,
            (struct sockaddr *)&addr, &addrlen);
    printf("[UDP]=(%s:%s)=============================================================\n", 
            inet_ntop(AF_INET, (struct sockaddr_in *) &addr.sin_addr, messageSender, INET_ADDRSTRLEN),  port);
    printf("%s\n", buffer);

    args = tokenize(buffer);

    if (!strcmp(args[0], "REG")) {
        messageToSend = processRegister(args);

        printf("status: %s", messageToSend);

    } else if (!strcmp(args[0], "PTP")) {
        messageToSend = processTopicPropose(args);

    } else if (!strcmp(args[0], "LTP\n")) {

        messageToSend = processTopicList(args);

    } else if (!strcmp(args[0], "LQU")) {

        messageToSend = processQuestionList(args);
    }

    printf("[Sending response]\n|%s|\n", messageToSend);
    n = sendto(fd, messageToSend, strlen(messageToSend), 0,
            (struct sockaddr *)&addr, addrlen);
    if(n == -1) {
        fatal(strerror(errno));
    }

    free(messageToSend);
    printf("[UDP]===================================================================================\n");
}

