#include "proxy.h"

char buf[MAXDATASIZE] = {0};
char d_buf[MAXDATASIZE] = {0};

char port[100] = {0};
char o_method[100] = {};
char o_URL[100] = {0};
char o_object[100] = {0};
char o_port[100] = {0};
int data_length = 0;
void segment_d(char *header)
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
        case 46:
            if (n == 1)
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
}
int sendall(int s, char *buf, int *len)
{
    int n = 0;
    int total = 0;        // how many bytes we've sent
    int bytesleft = *len; // how many we have left to send    int n;
    while (total < *len)
    {
        n = send(s, buf + total, bytesleft, 0);
        if (n == -1)
        {
            break;
        }
        total += n;
        bytesleft -= n;
    }
    *len = total;            // return number actually sent here
    return n == -1 ? -1 : 0; // return -1 on failure, 0 on success }
}
int recvall(int s, char *buf, int *len)
{
    int n = 0;
    int total = 0;        // how many bytes we've sent
    int bytesleft = *len; // how many we have left to send    int n;
    while (total < *len)
    {
        n = recv(s, buf + total, bytesleft, 0);
        if (n == -1)
        {
            break;
        }
        total += n;
        bytesleft -= n;
    }
    *len = total;            // return number actually sent here
    return n == -1 ? -1 : 0; // return -1 on failure, 0 on success }
}
int fr_ser(int sersock, int newfd, int len)
{
    int t;
    while (1)
    {
        t = recv(sersock, d_buf, len,0);
        if (t == 0)
        {
            shutdown(newfd, 1);
            return 0;
        } 
        if (t == -1)
        {
            return -1;
        }
        
        sendall(newfd, d_buf, &t);
    }
}
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
            strcpy(o_port, temp);
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
            //fprintf(stdout, "%s", temp);
            strcpy(o_method, temp);
            if (strcmp(o_method, "CONNECT") == 0)
            {
                n = 1;
            }
            break;
        case 2:
            strcpy(temp, pch);
            //fprintf(stdout, "%s", temp); //2
            strcpy(o_object, temp);
            //fflush(stdout);
            break;
        case 5:
            if (n == 0)
            {
                strcpy(temp, pch);
                //fprintf(stdout, "%s", temp); //5
                strcpy(o_URL, temp);
                //fflush(stdout);
            }

            break;
        case 6:
            if (n == 1)
            {
                strcpy(temp, pch);
                //fprintf(stdout, "%s", temp); //5
                strcpy(o_URL, temp);
                //fflush(stdout);
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
    int sersock;

    int p[2];

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    int pid;
    //pipe(p);
    /*     if ((pid = fork()) == 0)//>
    { */
    if(argc!=2)
    {
        printf("usage: add a port");
        return -1;
    }
    if ((status = getaddrinfo(NULL, argv[1], &hints, &res)) != 0)
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
    int n;
    while (1)
    {
        newfd = accept(sockfd, (struct sockaddr *)&their_addr, &addr_size);
        if (fork() == 0)
        {
            continue;
        }
        if ((status = recv(newfd, buf, MAXDATASIZE, 0)) > 0)
        {
            char pch[MAXDATASIZE];
            int i;
            //fprintf(stdout, " %s \n", buf);
            //fflush(stdout);
            strcpy(pch, buf);
            segment_h(pch);
            //fprintf(stdout, "%s", o_URL);
            //fprintf(stdout, "%s", o_object);
            segment(o_URL);
            if (*o_port == '\0')
            {
                strcpy(o_port, "80");
            }

            if ((status = getaddrinfo(o_URL, o_port, &hints, &res)) != 0)
            {
                fprintf(stderr, "getaddinfo: %s\n", gai_strerror(status));
                return 2;
            }
            sersock = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
            if ((status = connect(sersock, res->ai_addr, res->ai_addrlen)) == -1)
            {
                perror("connect error");
            }
            //GET / HTTP/1.0 Host: www.kaist.edu \r\n\r\n
            memset(buf, 0, strlen(buf));
            sprintf(buf,"%s %s HTTP/1.0\r\nHost: %s\r\n\r\n","GET",o_object,o_URL);
            //fprintf(stdout, "%s", buf);
            //fflush(stdout);
            send(sersock, buf, strlen(buf), 0);
            memset(buf, 0, strlen(buf));
            /* recv(sersock, buf, MAXDATASIZE, 0);

            send(newfd, buf, strlen(buf), 0);
            fprintf(stdout, "%s", buf);
            fflush(stdout); */

            int t = 0;
            int s = 0;
            int datasize = 0;
            // while(1)
            // {
            //     t = fr_ser(sersock, newfd, MAXDATASIZE);
            //     if(t==0||t==-1)
            //     {
            //         break;
            //     }
            // }
            while (1)
            {
                //datasize = (s > t) ? s - t : (MAXDATASIZE == t ? MAXDATASIZE : MAXDATASIZE - t);
                /* int len = MAXDATASIZE;
                t = recv(sersock, d_buf, len, 0);
                
                if (t == 0)
                {
                    sendall(newfd, d_buf, &t);
                    shutdown(newfd, 1);
                    //s=recv(newfd,d_buf,len,0);
                    break;
                }
                sendall(newfd, d_buf, &t);
                if(t==-1)
                {
                    break;
                }
                memset(d_buf, 0, MAXDATASIZE); */
                if (s != -1)
                {
                    t = fr_ser(sersock, newfd, MAXDATASIZE);
                }
                /* if (t != -1)
                {
                    s = fr_ser(newfd, sersock, MAXDATASIZE);
                } */
                if ( (t == 0)||t==-1/* || (t == -1) */)
                {
                    break;
                }
            }
        }
        close(sockfd);
        close(newfd);
        close(sersock);
        return 0;
    }
}