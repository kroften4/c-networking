#include <string.h>
#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <errno.h>
#include <wait.h>
#include <stdlib.h>
#include <unistd.h>

#define BACKLOG 10

void *get_in_addr(struct sockaddr_storage *ss)
{
    if (ss->ss_family == AF_INET) {
        return &(((struct sockaddr_in *)ss)->sin_addr);
    }
    return &(((struct sockaddr_in6 *)ss)->sin6_addr);
}

int main(int argc, char *argv[]) {
    int errno_save = 0, status, sockfd, connfd;
    struct addrinfo hints;
    struct addrinfo *res;

    if (argc != 2) {
        printf("usage: krftn_test_server <port>\n");
        return EXIT_FAILURE;
    }

    char *server_port = argv[1];

    // get addrinfo for host
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET; // ipv4 (TODO: make switches to choose)
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_addr = (struct sockaddr *)AI_PASSIVE;
    if ((status = getaddrinfo(NULL, server_port, &hints, &res)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(status));
        return EXIT_FAILURE;
    }

    // create a socket and bind
    struct addrinfo *p;
    for (p = res; p != NULL; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
            errno_save = errno;
            fprintf(stderr, "socket: %s\n", strerror(errno_save));
            continue;
        }
        
        if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            errno_save = errno;
            close(sockfd);
            fprintf(stderr, "bind: %s\n", strerror(errno_save));
            continue;
        }

        break; // successfull socket() and bind()
    }

    if (p == NULL) {
        perror("server: can't create a socket");
        return EXIT_FAILURE;
    }

    struct addrinfo server_ai = *p;
    freeaddrinfo(p);

    if (listen(sockfd, BACKLOG) == -1) {
        errno_save = errno;
        close(sockfd);
        perror("listen");
        return errno_save;
    }

    char server_address[INET6_ADDRSTRLEN];
    inet_ntop(server_ai.ai_family, get_in_addr((struct sockaddr_storage *)server_ai.ai_addr), server_address, server_ai.ai_addrlen);
    if (server_ai.ai_family == AF_INET) {
        printf("listening on %s:%s\n", server_address, server_port);
    }
    if (server_ai.ai_family == AF_INET6) {
        printf("listening on [%s]:%s\n", server_address, server_port);
    }

    // accept connections
    for (;;) {
        struct sockaddr_storage conn_addr;
        socklen_t conn_addrsize = sizeof conn_addr;
        if ((connfd = accept(sockfd, (struct sockaddr *) &conn_addr, &conn_addrsize)) == -1) {
            errno_save = errno;
            perror("accept");
            goto conn_cleanup;
        }

        int pid = fork();
        if (pid == -1) {
            errno_save = errno;
            perror("fork");
            goto conn_cleanup;
        }
        if (!pid) {
            close(sockfd);
            printf("some individual has connected\n");
            char test_msg[] = 
                "HTTP/1.0 200 OK\n"
                "Content-Type: text/html\n"
                "Content-Length: 105\n"
                "\n"
                "<!DOCTYPE html><html><head><title>C HTTP server</title></head><body>I am the one who serves</body></html>"
            ;
            ssize_t test_msglen = sizeof test_msg;
            ssize_t size_sent = send(connfd, test_msg, test_msglen, 0);
            if (size_sent == -1) {
                errno_save = errno;
                fprintf(stderr, "send: %s\n", strerror(errno_save));
                exit(errno_save);
            }
            printf("sent %zu/%zu bytes of data to some individual\n", size_sent, test_msglen);
            exit(0);
        }
        wait(NULL);

        conn_cleanup:
        close(connfd);
        if (errno_save != 0) {
            fprintf(stderr, "%s\n", strerror(errno_save));
        }
    }

    close(sockfd);
    return errno_save;
}
