#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>

#include <arpa/inet.h>


void readLineArgs(char* serverIP, char* port){

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
    if(n != 0) exit(1); 

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

    char serverIP[128], port[128], hostname[128];
    struct addrinfo hints_TCP, *res_TCP;
    struct addrinfo hints_UDP, *res_UDP;
    int n, flagToUse;

    char buffer[INET_ADDRSTRLEN];
    struct in_addr *addr;



    if(argc == 1){
        n = gethostname(hostname, 128);
        if(n == -1)  exit(1);
        strcpy(port, "58036");
        flagToUse = 0;

        setAddrStruct(hostname, port, &hints_TCP, &res_TCP, &hints_UDP, &res_UDP, flagToUse);


    } else {
        readLineArgs(serverIP, port);
        setAddrStruct(serverIP, port, &hints_TCP, &res_TCP, &hints_UDP, &res_UDP, flagToUse);
    }


    addr=&((struct sockaddr_in *) res_UDP->ai_addr)->sin_addr;
    printf("cenas: %s\n", inet_ntop(res_UDP->ai_family, addr, buffer, sizeof buffer));
    addr=&((struct sockaddr_in *) res_TCP->ai_addr)->sin_addr;
    printf("cenas: %s\n", inet_ntop(res_TCP->ai_family, addr, buffer, sizeof buffer));


}