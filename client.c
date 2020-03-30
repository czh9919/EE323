#include <stdio.h>
#include <unistd.h> 
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#define MYPORT "5700"
#define BACKLOG 10
#define MAXDATASIZE 100
char buf[MAXDATASIZE];

int main(int argc, char const *argv[])
{
    struct sockaddr_storage their_addr;
    socklen_t addr_size;
    struct addrinfo *res, *p;
    struct addrinfo hints;
    int status;
    char ipstr[INET6_ADDRSTRLEN];
    int sockfd, newfd;

    // if (argc != 2)
    // {
    //     fprintf(stderr,"usage: showip hostname\n");
    //     return 1;
    // }

    memset(&hints,0,sizeof(hints));
    hints.ai_family=AF_UNSPEC;
    hints.ai_socktype=SOCK_STREAM;
    hints.ai_flags=AI_PASSIVE;

    if((status=getaddrinfo("dbd039.cn","5700",&hints,&res))!=0)
    {
        fprintf(stderr,"getaddinfo: %s\n",gai_strerror(status));
        return 2;
    }

    for(p=res;p!=NULL;p=p->ai_next)
    {
        void *addr;
        char *ipver;
        if(p->ai_family==AF_INET)
        {
            struct sockaddr_in *ipv4 = (struct sockaddr_in *)p->ai_addr;
            addr=&(ipv4->sin_addr);
            ipver="IPv4";
        }
        else
        {
            struct sockaddr_in6 *ipv6 =(struct sockaddr_in6 *)p->ai_addr;
            addr= &(ipv6->sin6_addr);
            ipver="IPv6";
        }
        inet_ntop(p->ai_family,addr,ipstr,sizeof(ipstr));
        printf(" %s:%s\n",ipver,ipstr);
    }

    printf("begin make socket\n");

    sockfd=socket(res->ai_family,res->ai_socktype,res->ai_protocol);
    connect(sockfd,res->ai_addr,res->ai_addrlen);

    scanf("%s",buf);
    send(newfd,buf,strlen(buf),0);

    freeaddrinfo(res);
    return 0;

}