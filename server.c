#include<sys/types.h>
#include<arpa/inet.h>
#include<string.h>
#include<unistd.h>
#include<stdio.h>
#include<stdlib.h>
#include<sys/socket.h>
#include<stdlib.h>

void do_some(int);

int main(int argc, char* argv[]) {
    if(argc > 1) {
        fprintf(stderr, "error running program");
        exit(1);
    }

    // create a handler
    // pass address parameters
    // create the socket and start listening


    // accept incoming requests in a loop
    int fd = socket(AF_INET, SOCK_STREAM, 0);

    if(fd == -1) {
        fprintf(stderr, "error creating handle");
        exit(1);
    }
    

    // set socket reused addr to 1 (for true) so that the server uses 
    // the same address and port after a restart
    int val = 1;
    if(setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &val, sizeof(val)) == -1) {
        perror("Error setting socket optins.");
        close(fd);
        exit(1);
    }

    struct sockaddr_in addr = {};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(1234);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);

   if(bind(fd, (const struct sockaddr*) &addr, sizeof(addr)) != 0) {
        perror("Error binding socket");
        close(fd);
        exit(1);

   }

   if(listen(fd, SOMAXCONN) != 0) {
        perror("Error listening on socket");
        close(fd);
        exit(1);
   }

    while(1) {
        struct sockaddr_in client_addr = {};
        socklen_t addrlen = sizeof(client_addr);

        int connfd = accept(fd, (struct sockaddr*) &client_addr, &addrlen);

        if(connfd < 0) {
            perror("Error accepting connection");
            continue;
        }

        do_some(connfd);

        close(connfd);

    }
    return 0;
}

void do_some(int connfd) {
    char rbuf[64] = {};
    size_t n = read(connfd, &rbuf, sizeof(rbuf) - 1);

    if(n < 0) {
        perror("Error reading from connection.");
        return;
    }
    printf("client says %s\n", rbuf);

    char wbuf[] = "world";
    write(connfd, &wbuf, strlen(wbuf));
}