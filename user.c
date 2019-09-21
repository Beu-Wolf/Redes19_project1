#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>

#include <arpa/inet.h>

#define INPUT_SIZE 64
#define INP_NUM_ARGS 4

#define BUFFER_SIZE 128
#define DEFAULT_PORT "58036"

extern char *optarg;

enum flags {
    GET_BY_IP,
    GET_BY_NAME,
};

typedef struct service {
        char serverIP[BUFFER_SIZE];
        char port[BUFFER_SIZE];
} service;

typedef struct addressInfoSet {

    struct addrinfo hints_TCP;
    struct addrinfo* res_TCP;
    struct addrinfo hints_UDP;
    struct addrinfo* res_UDP;

} addressInfoSet;

enum flags flags;

void readLineArgs(int argc, char* argv[], service* newService){
    int n, opt;
    n = gethostname(newService->serverIP, BUFFER_SIZE);
    if(n == -1) exit(1);

    strcpy(newService->port, DEFAULT_PORT);
    flags = GET_BY_NAME;
    
    while((opt = getopt(argc, argv, "n:p:")) != -1) {
        switch (opt) {
        case 'n':
            strcpy(newService->serverIP, optarg);
            printf("%s\n", newService->serverIP);
            flags = GET_BY_IP;
            break;
        case 'p':
            strcpy(newService->port, optarg);
            printf("%s\n", newService->port);
            break;
        default:
            break;
        }
    }

}

void setAddrStruct(service* newService, addressInfoSet* newAddrInfoSet){

    int n;

    memset(&(newAddrInfoSet->hints_TCP), 0, sizeof(newAddrInfoSet->hints_TCP));
    newAddrInfoSet->hints_TCP.ai_family = AF_INET;
    newAddrInfoSet->hints_TCP.ai_socktype = SOCK_STREAM;
    newAddrInfoSet->hints_TCP.ai_protocol = IPPROTO_TCP;

    if(flags == GET_BY_NAME){
        newAddrInfoSet->hints_TCP.ai_flags = AI_NUMERICSERV;
    } else if (flags == GET_BY_IP) {
        newAddrInfoSet->hints_TCP.ai_flags = AI_NUMERICHOST;
    }

    n = getaddrinfo(newService->serverIP, newService->port, 
    &(newAddrInfoSet->hints_TCP), &(newAddrInfoSet->res_TCP));
    if(n != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(n));
        exit(1); 
    } 

    memset(&(newAddrInfoSet->hints_UDP), 0, sizeof(newAddrInfoSet->hints_UDP));
    newAddrInfoSet->hints_UDP.ai_family = AF_INET;
    newAddrInfoSet->hints_UDP.ai_socktype = SOCK_DGRAM;
    newAddrInfoSet->hints_UDP.ai_protocol = IPPROTO_UDP;


    if(flags == GET_BY_NAME){
        newAddrInfoSet->hints_UDP.ai_flags = AI_NUMERICSERV;
    } else if(flags == GET_BY_IP) {
        newAddrInfoSet->hints_UDP.ai_flags = AI_NUMERICHOST;
    }

     
    n = getaddrinfo(newService->serverIP, newService->port, 
    &(newAddrInfoSet->hints_UDP), &(newAddrInfoSet->res_UDP));
    if(n != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(n));
        exit(1); 
    }

   

}

// return list of inserted words (separated by ' ')
void readCommand(char** bufPtr, int* bufSize) {
  int i;
  char c;

  // read from stdin
  i = 0;
  while((c = getchar()) != '\n' && c != '\0' && c != EOF) {
    (*bufPtr)[i++] = c;
    if(i == *bufSize) {
      *bufSize += INPUT_SIZE;
      *bufPtr = (char*)realloc(*bufPtr, *bufSize * sizeof(char));
    }
  }
  if(c == EOF) {
    printf("Read EOF. Exiting\n");
    exit(1);
  }
  (*bufPtr)[i] = '\0';
}

// parse input (similar to strtok. prepared to ignore in-quote spaces)
char** tokenize(char* string) {
  int numArgs, i;
  char* currChar, *currArg;
  char** args;

  numArgs = INP_NUM_ARGS;
  args = (char**)malloc(INP_NUM_ARGS * sizeof(char*));
  if(args == NULL) exit(1);

  i = 0;
  currChar = currArg = string;
  while(*currChar != '\0'){
    if(*currChar == ' ') {
      *currChar = '\0';
      args[i++] = currArg;
      if(i == numArgs - 1) {
        numArgs += INP_NUM_ARGS;
        args = realloc(args, numArgs * sizeof(char*));
      }
      currArg = currChar+1;
    }
    currChar++;
  }
  args[i++] = currArg;
  args[i] = NULL;

  return args;
}


int main(int argc, char* argv[]) {

    char hostname[BUFFER_SIZE];
    char sendMsg[BUFFER_SIZE];

    int inpSize;
    char* input;
    char** parsedInput;

    service newService; 
    addressInfoSet newAddrInfoSet;

    int n;
    int fdTCP, fdUDP;

    ssize_t sentB, sendBLeft;

    char buffer[INET_ADDRSTRLEN];
    struct in_addr *addr;

    readLineArgs(argc, argv, &newService);
    setAddrStruct(&newService, &newAddrInfoSet);

    addr=&((struct sockaddr_in *) (newAddrInfoSet.res_UDP)->ai_addr)->sin_addr;
    printf("cenas: %s\n", inet_ntop((newAddrInfoSet.res_UDP)->ai_family, addr, buffer, sizeof buffer));
    addr=&((struct sockaddr_in *) (newAddrInfoSet.res_TCP)->ai_addr)->sin_addr;
    printf("cenas: %s\n", inet_ntop((newAddrInfoSet.res_TCP)->ai_family, addr, buffer, sizeof buffer));

    inpSize = INPUT_SIZE;
    input = (char*)malloc(inpSize * sizeof(char));
    if(input == NULL) exit(1);
    while(1) {
      readCommand(&input, &inpSize);
      printf("read: |%s|\n", input);
      parsedInput = tokenize(input);
      int i = 0;
      while(parsedInput[i] != NULL) {
        printf("[%d] -> %s\n", i, parsedInput[i]);
        i++;
      }

      free(parsedInput);
    }

    free(input);

    while(0) {
        fgets(sendMsg, BUFFER_SIZE, stdin);
        if(atoi(sendMsg) == 1) {
            fdUDP = socket(newAddrInfoSet.res_UDP->ai_family, 
            newAddrInfoSet.res_UDP->ai_socktype, newAddrInfoSet.res_UDP->ai_protocol);

            if(fdUDP == -1) exit(1);

            n = sendto(fdUDP, "TestUDP\n", 8, 0, newAddrInfoSet.res_UDP->ai_addr, 
            newAddrInfoSet.res_UDP->ai_addrlen);

            close(fdUDP);


        } else if(atoi(sendMsg) == 2){
            fdTCP = socket(newAddrInfoSet.res_TCP->ai_family, 
            newAddrInfoSet.res_TCP->ai_socktype, newAddrInfoSet.res_TCP->ai_protocol);
            if(fdTCP == -1) exit(1);

            n = connect(fdTCP, newAddrInfoSet.res_TCP->ai_addr, 
            newAddrInfoSet.res_TCP->ai_addrlen);
            if(n == -1) exit(1);

            strcpy(sendMsg, "Hello\n");
            sendBLeft = 7;
            while(sendBLeft > 0){
                sentB = write(fdTCP, sendMsg, sendBLeft);
                if(sentB <= 0) exit(1);

                sendBLeft -= sentB;

                close(fdTCP);
            }



        } else {
            printf("Error\n");
            break;
        }
    }

}
