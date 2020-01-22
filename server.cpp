/*
** server.c -- a stream socket server demo
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

int main(void)
{
    const char *PORT = "3000";
    const int BACKLOG = 1; // how many pending connections queue will hold

    int sockfd;  // listen on sock_fd
    int yes=1;
    int rv;

    struct addrinfo hints, *servinfo, *p;
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE; // use my IP

    if ((rv = getaddrinfo(NULL, PORT, &hints, &servinfo)) != 0) {
        std::cerr << "getaddrinfo: " << gai_strerror(rv) << std::endl;
        exit(1);
    }

    // loop through all the results and bind to the first we can
    for(p = servinfo; p != NULL; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype,
                p->ai_protocol)) == -1) {
            perror("server: socket");
            continue;
        }

        if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes,
                sizeof(int)) == -1) {
            perror("setsockopt");
            exit(1);
        }

        if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            close(sockfd);
            perror("server: bind");
            continue;
        }

        break;
    }

    freeaddrinfo(servinfo); // all done with this structure

    if (p == NULL)  {
        std::cerr << "server: failed to bind" << std::endl;
        exit(1);
    }

    if (listen(sockfd, BACKLOG) == -1) {
        perror("listen");
        exit(1);
    }

    std::cout << "server: waiting for connections..." << std::endl;

    int new_fd; // new connection
    struct sockaddr_storage their_addr; // connector's address information
    socklen_t sin_size;
    char s[INET_ADDRSTRLEN];

    while(1) {  // main accept() loop
        sin_size = sizeof(their_addr);
        new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size);
        if (new_fd == -1) {
            perror("accept");
            continue;
        }

        inet_ntop(their_addr.ss_family,
                get_in_addr((struct sockaddr *) &their_addr),
                s, sizeof(s));
        std::cout << "server: got connection from " << s << std::endl;

        if (send(new_fd, "Hello, world!", 13, 0) == -1)
            perror("send");

        close(new_fd);
    }

    return 0;
}
