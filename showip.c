#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <stdlib.h>
#include <stdio.h>

char *addrinfo_to_p(const struct addrinfo *ai, char *buf) {
    void *addr;
    switch (ai->ai_family) {
    case AF_INET:
    {
        struct sockaddr_in *in_addr;
        in_addr = (struct sockaddr_in *)(ai->ai_addr);
        addr = &(in_addr->sin_addr);
        break;
    }
    case AF_INET6:
    {
        struct sockaddr_in6 *in6_addr;
        in6_addr = (struct sockaddr_in6 *)(ai->ai_addr);
        addr = &(in6_addr->sin6_addr);
        break;
    }
    default:
        return NULL;
    }

    inet_ntop(ai->ai_family, addr, buf, ai->ai_addrlen);

    return buf;
}

int main(int argc, char **argv) {
    if (argc != 2) {
        printf("usage: showip <hostname>");
        exit(1);
    }

    char *hostname = argv[1];

    struct addrinfo *host_ai;
    int status = getaddrinfo(hostname, NULL, NULL, &host_ai);
    if (status != 0) {
        freeaddrinfo(host_ai);
        exit(1);
    }

    for ( ; host_ai->ai_next != NULL; host_ai = host_ai->ai_next) {
        char host_ip[INET6_ADDRSTRLEN];
        if (addrinfo_to_p(host_ai, host_ip) == NULL) {
            continue;
        }

        char *socktype;
        switch (host_ai->ai_socktype) {
        case SOCK_STREAM:
            socktype = "tcp";
            break;
        case SOCK_DGRAM:
            socktype = "udp";
            break;
        case SOCK_RAW:
            socktype = "raw";
            break;
        default:
            socktype = "unknown";
            break;
        }
        printf("%s: %s\n", socktype, host_ip);
    }

    freeaddrinfo(host_ai);
    return 0;
}
