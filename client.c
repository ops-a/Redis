#include<stdio.h>
#include<string.h>
#include<unistd.h>
#include<arpa/inet.h>
#include<stdlib.h>
#include<assert.h>
#include<sys/types.h>
#include<sys/socket.h>

const int k_max_msg = 4096;

int32_t write_all(int, char*, size_t);
int32_t read_full(int, char*, size_t);
int32_t query(int, const char*);

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
    int err = query(fd, msg);
    if(err) {
        perror("error occurred");
        goto LD;
    }

    err = query(fd, "hello2");
    if(err) {
        perror("error occurred");
        goto LD;
    }

    LD:
        close(fd);
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

int32_t read_full(int fd, char* buf, size_t n) {
    while(n > 0) {
        ssize_t rv = read(fd, buf, n);
        if(rv <= 0) {
            return -1;
        }

        assert((size_t) rv <= n);
        n -= rv;
        buf += rv;
    }

    return 0;
}

int32_t query(int fd, const char* text) {
    int len = strlen(text);
    if(len > k_max_msg) {
        perror("message too long");
        return -1;
    }

    char wbuf[4 + k_max_msg];
    // copy len and text into wbuf;
    memcpy(wbuf, &len, 4);
    memcpy(&wbuf[4], text, len);

    // write to the server
    int err = write_all(fd, wbuf, 4 + len);

    if(err) {
        perror("write() error");
        return err;
    }

    // Read the response
    char rbuf[4 + k_max_msg + 1];
    err = read_full(fd, rbuf, 4);

    if(err) {
        perror("reading from server: read() error");
        return err;
    }

    // copy the length of characters to be read
    memcpy(&len, rbuf, 4);
    if(len > k_max_msg) {
        perror("message too long");
        return -1;
    }

    // get the response
    err = read_full(fd, &rbuf[4], len);

    // print the response
    printf("server says: %.*s\n", len, &rbuf[4]);

    return 0;
}