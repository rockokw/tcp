/*
** client.c -- a stream socket client demo
*/

#include <iostream>

#include <assert.h>
#include <errno.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>

// get sockaddr, IPv4:
void *get_in_addr(struct sockaddr *sa)
{
    assert(sa->sa_family == AF_INET);
    return &(((struct sockaddr_in*)sa)->sin_addr);
}

int main(int argc, char *argv[])
{
    const char *ADDR = "127.0.0.1"; // the IP address to connect to
    const char *PORT = "3000"; // the port client will be connecting to 
    const int MAXDATASIZE = 100; // max number of bytes we can get at once 

    int sockfd, numbytes;

    /*
    if (argc != 2) {
        fprintf(stderr,"usage: client hostname\n");
        exit(1);
    }
    */

    struct addrinfo hints, *servinfo, *p;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;

    int rv;
    if ((rv = getaddrinfo(ADDR, PORT, &hints, &servinfo)) != 0) {
        std::cerr << "getaddrinfo: " << gai_strerror(rv) << std::endl;
        exit(1);
    }

    // loop through all the results and connect to the first we can
    for(p = servinfo; p != NULL; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype,
                p->ai_protocol)) == -1) {
            perror("client: socket");
            continue;
        }

        if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            perror("client: connect");
            close(sockfd);
            continue;
        }

        break;
    }

    if (p == NULL) {
        std::cerr << "client: failed to connect" << std::endl;
        exit(2);
    }

    char s[INET_ADDRSTRLEN];
    inet_ntop(p->ai_family, get_in_addr((struct sockaddr *)p->ai_addr),
            s, sizeof s);
    std::cout << "client: connecting to " <<  s << std::endl;

    freeaddrinfo(servinfo); // all done with this structure

    char buf[MAXDATASIZE];
    if ((numbytes = recv(sockfd, buf, MAXDATASIZE-1, 0)) == -1) {
        perror("recv");
        exit(1);
    }

    buf[numbytes] = '\0';

    printf("client: received '%s'\n",buf);

    close(sockfd);

    return 0;
}
