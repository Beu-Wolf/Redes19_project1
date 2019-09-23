#include "util.h"
#include <stdlib.h>
#include <stdio.h>
#include <sys/socket.h>
#include <string.h>

#define INPUT_SIZE 128

void fatal(char* buffer) {
  printf("%s\n", buffer);
  exit(1);
}

char **tokenize(char *string) {
	int numArgs = 10;
	int i = 0;

	char **args = (char **)malloc(numArgs * sizeof(char *));
	if (!args) fatal("Allocation error");

	args[i] = strtok(string, " ");

	while (args[i] != NULL) {
		i++;
		if (i == numArgs) {
			numArgs *= 2;
			args = realloc(args, numArgs * sizeof(char *));
			if (!args) fatal("Allocation error");
		}
		args[i] = strtok(NULL, " ");
	}

	return args;
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

/* Acceps socket FD, a buffer and its size
 * Reads from socket until '\n' read. reallocates buffer if needed
 * Returns number of bytes read
 */
int recvTCPline(int sockfd, char** buffer, int* size) {
  char* ptr = *buffer;

  // TODO: Is this really necessary???
  // allocate buffer if needed
  if(*buffer == NULL || *size == 0) {
    (*buffer) = (char*)malloc(INPUT_SIZE * sizeof(char));
    if(!(*buffer)) {
      fatal("Allocation error");
    }
    *size = INPUT_SIZE;
  }

  while(recv(sockfd, ptr, 1, 0) == 1) {
    if(*(ptr++) == '\n')
      break; // terminate string and return

    if(ptr - (*buffer) >= *size) { // resize buffer
      *buffer = (char*)realloc(*buffer, 2 * *size);
      if(!(*buffer))
        fatal("Allocation error\n");
      (*size) *= 2;
    }
  }
  *ptr = '\0';
  return strlen(*buffer);
}
