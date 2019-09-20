#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>

#include <arpa/inet.h>

#define BUFFER_SIZE 128

extern char *optarg;


void readLineArgs(int argc, char* argv[], char* serverIP, char* port, int *flagToUse){
    int n, opt;
    n = gethostname(serverIP, BUFFER_SIZE);
    if(n == -1) exit(1);

    strcpy(port, "58036");
    *flagToUse = 0;
    
    while((opt = getopt(argc, argv, "n:p:")) != -1) {
        switch (opt) {
        case 'n':
            strcpy(serverIP, optarg);
            printf("%s\n", serverIP);
            *flagToUse = 1;
            break;
        case 'p':
            strcpy(port, optarg);
            printf("%s\n", port);
            break;
        default:
            break;
        }
    }

}

void setAddrStruct(char* hostname, char* port, struct addrinfo* hints_TCP,
 struct addrinfo** res_TCP, struct addrinfo* hints_UDP, 
 struct addrinfo** res_UDP, int flagToUse){

    int n;

    memset(hints_TCP, 0, sizeof(hints_TCP));
    hints_TCP->ai_family = AF_INET;
    hints_TCP->ai_socktype = SOCK_STREAM;

    if(!flagToUse){
        hints_TCP->ai_flags = AI_NUMERICSERV;
    } else {
        hints_TCP->ai_flags = AI_NUMERICHOST;
    }
    


    n = getaddrinfo(hostname, port, hints_TCP, res_TCP);
    if(n != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(n));
        exit(1); 
    } 

    memset(hints_UDP, 0, sizeof(hints_UDP));
    hints_UDP->ai_family = AF_INET;
    hints_UDP->ai_socktype = SOCK_DGRAM;
    hints_UDP->ai_protocol = IPPROTO_UDP;


    if(!flagToUse){
        hints_UDP->ai_flags = AI_NUMERICSERV;
    } else {
        hints_UDP->ai_flags = AI_NUMERICHOST;
    }

     
    n = getaddrinfo(hostname, port, hints_UDP, res_UDP);
    if(n != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(n));
        exit(1); 
    }

   

}



int main(int argc, char* argv[]) {

    char serverIP[BUFFER_SIZE];
    char port[BUFFER_SIZE];
    char hostname[BUFFER_SIZE];
    char sendMsg[BUFFER_SIZE];

    struct addrinfo hints_TCP, *res_TCP;
    struct addrinfo hints_UDP, *res_UDP;

    int n, flagToUse;
    int fdTCP, fdUDP;

    ssize_t sentB, sendBLeft;

    char buffer[INET_ADDRSTRLEN];
    struct in_addr *addr;

    readLineArgs(argc, argv, serverIP, port, &flagToUse);
    setAddrStruct(serverIP, port, &hints_TCP, &res_TCP, &hints_UDP, &res_UDP, flagToUse);

    addr=&((struct sockaddr_in *) res_UDP->ai_addr)->sin_addr;
    printf("cenas: %s\n", inet_ntop(res_UDP->ai_family, addr, buffer, sizeof buffer));
    addr=&((struct sockaddr_in *) res_TCP->ai_addr)->sin_addr;
    printf("cenas: %s\n", inet_ntop(res_TCP->ai_family, addr, buffer, sizeof buffer));


    while(1) {
        fgets(sendMsg, BUFFER_SIZE, stdin);
        if(atoi(sendMsg) == 1) {
            fdUDP = socket(res_UDP->ai_family, res_UDP->ai_socktype, res_UDP->ai_protocol);

            if(fdUDP == -1) exit(1);

            n = sendto(fdUDP, "TestUDP\n", 8, 0, res_UDP->ai_addr, res_UDP->ai_addrlen);

            close(fdUDP);


        } else if(atoi(sendMsg) == 2){
            fdTCP = socket(res_TCP->ai_family, res_TCP->ai_socktype, res_TCP->ai_protocol);
            if(fdTCP == -1) exit(1);

            n = connect(fdTCP, res_TCP->ai_addr, res_TCP->ai_addrlen);
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
