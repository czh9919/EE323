#include "proxy.h"

char buf[MAXDATASIZE] = {0};

char port[100] = {0};
char o_method[100] = {};
char o_URL[100] = {0};
char o_object[100] = {0};

void segment(char *URL)
{
    char *pch;
    char str[MAXDATASIZE];
    int i = 0;
    strcpy(str, URL);
    pch = strtok(str, ":");
    i++;
    char temp[100];
    while (pch != NULL)
    {
        switch (i)
        {
        case 1:
            strcpy(temp, pch);
            fprintf(stdout, "%s", temp);
            strcpy(o_URL, temp);
            break;
        case 2:
            strcpy(temp, pch);
            fprintf(stdout, "%s", temp); //2
            strcpy(port, temp);
            fflush(stdout);
            break;
        default:
            break;
        }
        pch = strtok(NULL, ":");
        i++;
    }
};

void segment_h(char *header)
{
    char *pch;
    int i = 0;
    int n = 0;
    char str[MAXDATASIZE];
    strcpy(str, header);
    pch = strtok(str, " \r\n");
    i++;
    char temp[100];
    while (pch != NULL)
    {
        switch (i)
        {
        case 1:
            strcpy(temp, pch);
            fprintf(stdout, "%s", temp);
            strcpy(o_method, temp);
            if (strcmp(o_method, "CONNECT")==0)
            {
                n = 1;
            }
            break;
        case 2:
            strcpy(temp, pch);
            fprintf(stdout, "%s", temp); //2
            strcpy(o_object, temp);
            fflush(stdout);
            break;
        case 5:
            if (n == 1)
            {
                strcpy(temp, pch);
                fprintf(stdout, "%s", temp); //5
                strcpy(o_URL, temp);
                fflush(stdout);
            }
            
            break;
        case 6:
            if(n==0)
            {
                strcpy(temp, pch);
                fprintf(stdout, "%s", temp); //5
                strcpy(o_URL, temp);
                fflush(stdout);
            }
            break;
        default:
            break;
        }
        pch = strtok(NULL, " \r\n");
        i++;
    }
};

int main(int argc, char const *argv[])
{
    struct sockaddr_storage their_addr;
    socklen_t addr_size;
    struct addrinfo *res;
    struct addrinfo hints;
    int status;
    char ipstr[INET6_ADDRSTRLEN];
    int sockfd, newfd;

    int p[2];

    /*     if (argc < 3)
    {
        fprintf(stderr, "lost arguement\n");
        return 1;
    }
    if (check_options(argc, argv) == ERROR)
    {
        fprintf(stderr, "arguement wrong\n");
        return 1;
    } */
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    int pid;
    pipe(p);
    if ((pid = fork()) == 0)//>
    {
        strcpy(port, "5711");
        if ((status = getaddrinfo(NULL, port, &hints, &res)) != 0)
        {
            fprintf(stderr, "getaddinfo: %s\n", gai_strerror(status));
            return 2;
        }
        sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
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
        //printf("perpare to serve\n");
        int n;
        while (1)
        {
            // printf("hello world\n");
            // printf("waiting for accept:\n");
            newfd = accept(sockfd, (struct sockaddr *)&their_addr, &addr_size);
            /* if (fork() == 0)
            {
                continue;
            } */

            while ((status = recv(newfd, buf, MAXDATASIZE, 0)) > 0)
            {
                char pch[MAXDATASIZE];
                int i;
                fprintf(stdout, " %s ", buf);
                fflush(stdout);
                strcpy(pch, buf);
                segment_h(pch);
                close(p[0]);
                write(p[1], o_method, 100);
                write(p[1], o_URL, 100);
                write(p[1], o_object, 100);
                //write(p[1],buf,MAXDATASIZE);
                printf("\n");
                // n = send(newfd, "1", MAXDATASIZE, 0);
                // if (n == -1)
                // {
                //     perror("recv wrong");
                //     continue;
                // }
            }
            printf("\n");
        }
    }
    else
    {
        close(p[1]);
        read(p[0], o_method, 100);
        read(p[0], o_URL, 100);
        read(p[0], o_object, 100);
        //read(p[0],buf,MAXDATASIZE);
        fprintf(stdout, "%s", o_URL);
        fprintf(stdout, "%s", o_object);
        segment(o_URL);
        if (*port == '\0')
        {
            strcpy(port, "80");
        }
        if (strcmp(o_method, "Get")==0)
        {
            if ((status = getaddrinfo(o_URL, port, &hints, &res)) != 0)
            {
                fprintf(stderr, "getaddinfo: %s\n", gai_strerror(status));
                return 2;
            }
            sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
            if ((status = connect(sockfd, res->ai_addr, res->ai_addrlen)) == -1)
            {
                perror("connect error");
            }
            send(sockfd, o_method, strlen(o_method), 0);
            send(sockfd, " ", 2, 0);
            send(sockfd, o_object, strlen(o_object), 0);
            send(sockfd, "HTTP/1.1\r\nHost: ", 17, 0);
            send(sockfd, o_URL, strlen(o_URL), 0);
            send(sockfd, "\r\n", 3, 0);
            send(sockfd, "Connection: keep-alive\r\n\n", 26, 0);
            //send(sockfd,"");
            recv(sockfd, buf, MAXDATASIZE, 0);
            fprintf(stdout, "%s", buf);

            fflush(stdout);
        }
        else if (strcmp(o_method, "CONNECT"))
        {
            if ((status = getaddrinfo(o_URL, port, &hints, &res)) != 0)
            {
                fprintf(stderr, "getaddinfo: %s\n", gai_strerror(status));
                return 2;
            }
            sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
            if ((status = connect(sockfd, res->ai_addr, res->ai_addrlen)) == -1)
            {
                perror("connect error");
            }
            read(p[0], buf, MAXDATASIZE);
            fprintf(stdout, "%s", buf);
            fflush(stdout);
            send(sockfd, buf, MAXDATASIZE, 0);
            recv(sockfd, buf, MAXDATASIZE, 0);
            fprintf(stdout, "%s", buf);
            fflush(stdout);
        }
    }

    freeaddrinfo(res);
    close(sockfd);
    return 0;
}