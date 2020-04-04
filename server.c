#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#define BACKLOG 10
#define MAXDATASIZE 100
#define ERROR -1
char buf[MAXDATASIZE] = {0};

char port[100] = {0};

int check_options(int argc, char const *argv[])
{
    int p_check = 0;
    for (int i = 1; i < argc; i += 2)
    {
        const char *p = argv[i];
        const char *s = argv[i + 1];
        if ((*p) != '-')
        {
            return ERROR;
        }
        while (*(++p))
        {
            switch (*p)
            {
            case ('p'):
                strcpy(port, s);
                p_check = 1;
                break;
            default:
                return ERROR;
            }
        }
    }
    if (p_check != 1)
    {
        return ERROR;
    }
    return 1;
}

int main(int argc, char const *argv[])
{
    struct sockaddr_storage their_addr;
    socklen_t addr_size;
    struct addrinfo *res, *p;
    struct addrinfo hints;
    int status;
    char ipstr[INET6_ADDRSTRLEN];
    int sockfd, newfd;

    if (argc < 3)
    {
        fprintf(stderr, "lost arguement\n");
        return 1;
    }
    if (check_options(argc, argv) == ERROR)
    {
        fprintf(stderr, "arguement wrong\n");
        return 1;
    }
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    if ((status = getaddrinfo(NULL, port, &hints, &res)) != 0)
    {
        fprintf(stderr, "getaddinfo: %s\n", gai_strerror(status));
        return 2;
    }

    for (p = res; p != NULL; p = p->ai_next)
    {
        void *addr;
        char *ipver;
        if (p->ai_family == AF_INET)
        {
            struct sockaddr_in *ipv4 = (struct sockaddr_in *)p->ai_addr;
            addr = &(ipv4->sin_addr);
            ipver = "IPv4";
        }
        else
        {
            struct sockaddr_in6 *ipv6 = (struct sockaddr_in6 *)p->ai_addr;
            addr = &(ipv6->sin6_addr);
            ipver = "IPv6";
        }
        inet_ntop(p->ai_family, addr, ipstr, sizeof(ipstr));
        printf(" %s:%s\n", ipver, ipstr);
    }

    printf("begin make socket\n");

    sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);

    printf("begin bind\n");

    if (bind(sockfd, res->ai_addr, res->ai_addrlen) == -1)
    {
        close(sockfd);
        perror("bind error");
        return 2;
    }
    if (listen(sockfd, BACKLOG) == -1)
    {
        close(sockfd);
        perror("listen error");
        return 2;
    }
    addr_size = sizeof(their_addr);
    printf("perpare to serve\n");

    while (1)
    {
        // printf("hello world\n");
        printf("waiting for accept:\n");
        newfd = accept(sockfd, (struct sockaddr *)&their_addr, &addr_size);
        if (fork() == 0)
        {
            continue;
        }
        while ((status = recv(newfd, buf, MAXDATASIZE, 0)) > 0)
        {
            fprintf(stdout,"%s", buf);
            fflush(stdout);
        }
        printf("\n");
    }
    freeaddrinfo(res);
    close(sockfd);
    return 0;
}