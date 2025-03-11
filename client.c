#include<stdio.h>
#include<string.h>
#include<unistd.h>
#include<arpa/inet.h>
#include<stdlib.h>
#include<sys/types.h>
#include<sys/socket.h>

int main(int argc, char* argv[]) {
    if(argc > 1) {
        perror("more than one argument");
        exit(1);
    }

    int fd = socket(AF_INET, SOCK_STREAM, 0);

    if(fd < 0) {
        perror("Error creating socket");
        exit(1);
    }

    struct sockaddr_in addr = {};
    addr.sin_family = AF_INET;
    addr.sin_port = ntohs(1234);
    addr.sin_addr.s_addr = ntohl(INADDR_LOOPBACK);

    if(connect(fd, (struct sockaddr*) &addr, sizeof(addr))) {
        perror("Error connecting to server\n");
        exit(1);
    }

    char msg[] = "hello";
    write(fd, msg, strlen(msg));

    char rbuf[64] = {};
    ssize_t n = read(fd, rbuf, sizeof(rbuf) - 1);
    if(n < 0) {
        perror("Error reading from server\n");
    }

    printf("server says: %s\n", rbuf);
    close(fd);
    return 0;
}