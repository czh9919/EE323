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
char buf[MAXDATASIZE];

#define error -1

char hostname[100]={0};
char port[100]={0};

int check_options(int argc,char const *argv[])
{
    int p_check=0;
    int h_check=0;
    for (int i = 1;i < argc; i+=2)
    {
        const char *p=argv[i];
        const char *s=argv[i+1];
        if((*p)!='-')
        {
            return error;
        }
        while(*(++p))
        {
            switch(*p)
            {
                case('h'):
                    strcpy(hostname,s);
                    h_check=1;
                    break;
                case('p'):
                    strcpy(port,s);
                    p_check=1;
                    break;
                default:
                    return error;
            }
        }
    }
    if(p_check!=1||h_check!=1)
    {
        return error;
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

    if (argc != 5)
    {
        fprintf(stderr,"lose argument\n");
        return 1;
    }

    if(check_options(argc,argv)==error)
    {
        fprintf(stderr,"argument wrong\n");
        return 1;
    }

    memset(&hints,0,sizeof(hints));
    hints.ai_family=AF_UNSPEC;
    hints.ai_socktype=SOCK_STREAM;

    strcpy(buf,"hello");


    if((status=getaddrinfo(hostname,port,&hints,&res))!=0)
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
    if((status=connect(sockfd,res->ai_addr,res->ai_addrlen))==-1)
    {
        perror("connect error");
    }


    send(sockfd,buf,strlen(buf),0);

    freeaddrinfo(res);
    return 0;

}