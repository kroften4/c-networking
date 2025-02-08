#include <string.h>
#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>

#define SERVER_NAME "localhost"
#define SERVER_PORT "3490"
#define BUF_LEN 1000

int main(int argc, char *argv[]) {
    int errnosave = 0, status, sockfd, connfd;
    struct addrinfo hints;
    struct addrinfo *res;

    // get addrinfo for host
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_addr = (struct sockaddr *)AI_PASSIVE;
    if ((status = getaddrinfo(NULL, SERVER_PORT, &hints, &res)) != 0) {
        fprintf(stderr, "getaddrinfo error: %s\n", gai_strerror(status));
        return EXIT_FAILURE;
    }

    // select the appropriate addrinfo linked list member
    if (res == NULL) {
        perror("couldn't find host addrinfo (list is empty)");
        return EXIT_FAILURE;
    }
    struct addrinfo server_ai = *res;
    freeaddrinfo(res);

    // make a socket and connect
    if ((sockfd = socket(server_ai.ai_family, server_ai.ai_socktype, server_ai.ai_protocol)) == -1) {
        errnosave = errno;
        perror("failed to create a socket");
        goto cleanup;
    }

    if (connect(sockfd, server_ai.ai_addr, server_ai.ai_addrlen) == -1) {
        errnosave = errno;
        perror("failed to connect");
        goto cleanup;
    }
    printf("connected to server\n");

    char buf[BUF_LEN];
    if (recv(sockfd, buf, sizeof buf, 0) == -1) {
        errnosave = errno;
        perror("fail in recv()");
        goto cleanup;
    }
    printf("recieved %s\n", buf);

    cleanup:
    close(sockfd);
    return errnosave;
}
