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

#define DEFAULT_PORT "58036"
#define BUFFER_SIZE 256

#define MAX(A,B) ((A)>= (B) ? (A):(B))

void receiveConnections(char *port);
int setupServerSocket(char *port, int socktype);

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

	//TODO: remove maybe
	char buffer[BUFFER_SIZE];
	struct sockaddr_in addr;
	socklen_t addrlen;
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
		if (counter <= 0) exit(1);

		if (FD_ISSET(udpSocket, &rfds)) {
			memset(buffer, 0, BUFFER_SIZE);
			recvfrom(udpSocket, buffer, BUFFER_SIZE, 0,
					(struct sockaddr *)&addr, &addrlen);
			printf("%s\n", buffer);
		}

		if (FD_ISSET(tcpSocket, &rfds)) {
			memset(buffer, 0, BUFFER_SIZE);
			newfd = accept(tcpSocket, (struct sockaddr *)&addr,
					&addrlen);
			if (newfd == -1) exit(1);

			read(newfd, buffer, BUFFER_SIZE);
			printf("%s\n", buffer);
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
