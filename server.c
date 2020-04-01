#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <assert.h>

#define NDEBUG

#define MYPORT "5700"
#define BACKLOG 10
#define MAXDATASIZE 100
char buf[MAXDATASIZE]={0};

int main(int argc, char const *argv[])
{
    struct sockaddr_storage their_addr;
    socklen_t addr_size;
    struct addrinfo *res, *p;
    struct addrinfo hints;
    int status;
    char ipstr[INET6_ADDRSTRLEN];
    int sockfd, newfd;

    char hostname[100];
    // if (argc != 2)
    // {
    //     fprintf(stderr,"usage: showip hostname\n");
    //     return 1;
    // }

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    if ((status = getaddrinfo(NULL, "5700", &hints, &res)) != 0)
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
    
    
    for(int i=0; i<10;i++)
    {
        // printf("hello world\n");
        printf("waiting for accept:\n");
        newfd = accept(sockfd, (struct sockaddr *)&their_addr, &addr_size);
        if(fork()!=0)
        {
            continue;
        }
        if ((status = recv(newfd, buf, MAXDATASIZE, 0)) == 0)
        {
            perror("connection lose");
            break;
        }
        if(buf[0]!='\0')
        {
            puts(buf);
            printf("success");
        }
            
    }
    freeaddrinfo(res);
    return 0;
}