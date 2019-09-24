#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <sys/time.h>
#include <netdb.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <errno.h>

#include "util.h"

#define DEFAULT_PORT "58036"
#define BUFFER_SIZE 1024

#define MAX(A,B) ((A)>= (B) ? (A):(B))

void receiveConnections(char *port);
int setupServerSocket(char *port, int socktype);
void handleTcp(int fd, char* port);
void handleUdp(int fd, char*port);
char* processRegister(char** tokenizedMessage);

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
			exit(1);
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
	int ret;

	int newfd;

	struct sockaddr_in addr;
	socklen_t addrlen;

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
		if (counter <= 0) exit(1);

		if (FD_ISSET(udpSocket, &rfds)) {
			handleUdp(udpSocket, port);
		}

		if (FD_ISSET(tcpSocket, &rfds)) {
			newfd = accept(tcpSocket,
					(struct sockaddr *)&addr,
					&addrlen);
			if (newfd == -1) exit(1);

			handleTcp(newfd, port);
		}
	}
}

int setupServerSocket(char *port, int socktype) {
	int fd;
	int n;
	struct addrinfo hints, *res;
	struct sockaddr_in addr;

	memset(&hints, 0, sizeof(struct addrinfo));
	hints.ai_family = AF_INET;
	hints.ai_socktype = socktype;
	hints.ai_flags = AI_PASSIVE | AI_NUMERICSERV;

	n = getaddrinfo(NULL, port, &hints, &res);
	if (n != 0) exit(1);

	fd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
	if (fd == -1) exit(1);

	n = bind(fd, res->ai_addr, res->ai_addrlen);
	if (n == -1) exit(1);

	if (socktype == SOCK_STREAM) {
		listen(fd, 5);
	}

	return fd;
}

void handleTcp(int fd, char* port) {
	int pid, ret;
	char buffer[BUFFER_SIZE];

	ret = fork();
	if (ret != 0) {
		if (ret == -1) {
			exit(-1);
		} else { // parent process
			close(fd);
			return;
		}
	}

	pid = getpid();
	printf("Forked. PID = %d\n", pid);
	while (1) {
		memset(buffer, 0, BUFFER_SIZE);
		if (read(fd, buffer, BUFFER_SIZE) == 0) {
			exit(0);
		}
		printf("[TCP][%d] %s\n", pid, buffer);
	}
}

void handleUdp(int fd, char* port) {
	char buffer[BUFFER_SIZE];
	char messageSender[INET_ADDRSTRLEN];

	char* messageToSend;

	char** tokenizedMessage;
	struct sockaddr_in addr;
	socklen_t addrlen = sizeof addr;

    memset(&addr, 0, sizeof(addr));


    int n;

	memset(buffer, 0, BUFFER_SIZE);
	recvfrom(fd, buffer, BUFFER_SIZE, 0,
			(struct sockaddr *)&addr, &addrlen);
	printf("[UDP] %s", buffer);
	
	tokenizedMessage = tokenize(buffer);

	if (!strcmp(tokenizedMessage[0], "REG")) {
		printf("Register request sent by: %s at %s\n", inet_ntop(AF_INET, (struct sockaddr_in *) &addr.sin_addr,
		messageSender, INET_ADDRSTRLEN),  port);
		messageToSend = processRegister(tokenizedMessage);   

        printf("status: %s", messageToSend);    

	}

    n = sendto(fd, messageToSend, strlen(messageToSend), 0, (struct sockaddr *)&addr, addrlen);
    if(n == -1) {
        printf("error message: %s\n", strerror(errno));
        exit(1);
    }



}

char* processRegister(char** tokenizedMessage) {
    char* registerStatus = (char *)malloc(BUFFER_SIZE * sizeof(char));
    if (!registerStatus) exit(1);

    errno = 0;

    if(tokenizedMessage[1] == NULL) {
        strcpy(registerStatus, "ERR\n");
        return registerStatus;
    }

    int number = strtol(tokenizedMessage[1], NULL, 0);
    if(errno == EINVAL) {
        strcpy(registerStatus, "ERR\n");
        return registerStatus;
    }
    
    if(number >= 10000 && number <= 99999) {
        strcpy(registerStatus, "RGR OK\n");
    } else {
        strcpy(registerStatus, "RGR NOK\n");
    }

    return registerStatus;

}
