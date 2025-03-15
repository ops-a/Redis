#include<sys/types.h>
#include<arpa/inet.h>
#include<string.h>
#include<unistd.h>
#include<stdio.h>
#include<stdlib.h>
#include<sys/socket.h>
#include<assert.h>
#include<stdlib.h>

void do_some(int);
int32_t one_request(int);
int32_t read_full(int, char*, size_t);
int32_t write_full(int, char*, size_t);

const size_t k_max_msg = 4096;

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

        while(1) {
            int32_t err = one_request(connfd);
            if(err) {
                break;
            }
        }

        close(connfd);

    }
    return 0;
}

int32_t  read_full(int fd, char* buf, size_t n) {
    while(n > 0) {
        int rv = read(fd, buf, n);
        if(rv <= 0) {
            return -1; // error
        }

        assert((size_t)rv <= n);
        n -= rv;
        buf += rv;
    }

    return 0;
}

int32_t write_all(int fd, char* buf, size_t n) {
    while(n > 0) {
        ssize_t rv = write(fd, buf, n);
        if(rv <= 0) {
            return -1;
        }

        assert((size_t)rv <= n);
        n -= rv;
        buf += rv;
    }

    return 0;
}

int32_t one_request(int connfd) {
    char rbuf[4 + k_max_msg];
    int errno = 0;

    int32_t err = read_full(connfd, rbuf, 4);
    if(err) {
        perror(errno == 0 ? "EOF" : "read() error");
        return err;
    }

    uint32_t len = 0;
    memcpy(&len, rbuf, 4);
    if(len > k_max_msg) {
        perror("Message too long");
        return -1;
    }

    // request body
    err = read_full(connfd, &rbuf[4], len);
    if(err) {
        perror("read() error");
        return err;
    }

    printf("client says: %.*s\n", len, &rbuf[4]);

    // reply using the same protocol
    const char reply[] = "world";
    char wbuf[4 + sizeof(reply)];
    len = (uint32_t) strlen(reply);
    memcpy(wbuf, &len, 4);
    memcpy(&wbuf[4], reply, len);

    return write_all(connfd, wbuf, 4 + len);


}
