#include <string.h>
#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>

#define SERVER_PORT "3490"
#define BACKLOG 10

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

    // make a socket, bind, and listen
    if ((sockfd = socket(server_ai.ai_family, server_ai.ai_socktype, server_ai.ai_protocol)) == -1) {
        errnosave = errno;
        perror("failed to create a socket");
        goto cleanup;
    }
    
    if (bind(sockfd, server_ai.ai_addr, server_ai.ai_addrlen) == -1) {
        errnosave = errno;
        perror("failed to bind socket");
        goto cleanup;
    }

    if (listen(sockfd, BACKLOG) == -1) {
        errnosave = errno;
        perror("failed to listen on socket FD");
        goto cleanup;
    }
    char server_address[INET6_ADDRSTRLEN];
    struct sockaddr_in *sa_in = (struct sockaddr_in *)server_ai.ai_addr;
    inet_ntop(server_ai.ai_family, &sa_in->sin_addr, server_address, server_ai.ai_addrlen);
    printf("listening on %s:%s\n", server_address, SERVER_PORT);

    struct sockaddr_storage conn_addr;
    socklen_t conn_addrsize;
    if ((connfd = accept(sockfd, (struct sockaddr *) &conn_addr, &conn_addrsize)) == -1) {
        errnosave = errno;
        perror("failed to accept connection");
        goto cleanup;
    }
    printf("some individual has connected\n");

    char test_msg[] = "Hello C Networking!";
    ssize_t test_msglen = sizeof test_msg;
    printf("test message: %s\ntest message size: %zu", test_msg, test_msglen);
    ssize_t size_sent = send(connfd, test_msg, test_msglen, 0);
    if (size_sent == -1) {
        errnosave = errno;
        perror("fail in send()");
        goto cleanup;
    }
    printf("sent %zu/%zu of data to some individual\n", size_sent, test_msglen);

    cleanup:
    close(connfd);
    close(sockfd);
    return errnosave;
}
