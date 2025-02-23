#include "proxy.h"

char buf[MAXDATASIZE] = {0};
char d_buf[MAXDATASIZE] = {0};

char port[100] = {0};
char o_method[100] = {};
char o_URL[100] = {0};
char o_object[100] = {0};
char o_port[100] = {0};
int data_length = 0;
void cr_link_list()
{
    int len;
    struct node_URL *current;
    struct node_URL *prev;
    char URL_buf[MAXDATASIZE];
    memset(URL_buf, 0, sizeof(URL_buf));
    if (!isatty(fileno(stdin)))
    {
        while (fgets(URL_buf, MAXDATASIZE, stdin) != NULL)
        {
            if (*URL_buf == '\0')
            {
                break;
            }
            if (head == NULL)
            {
                head = (struct node_URL *)malloc(sizeof(struct node_URL));
                current = head;
            }
            else
            {
                current = (struct node_URL *)malloc(sizeof(struct node_URL));
                prev->next = current;
            }
            char temp[MAXDATASIZE];
            if (strncmp(URL_buf, "http://", 7) == 0)
            {
                strncpy(temp, URL_buf + 7, sizeof(URL_buf));
                strncpy(URL_buf, temp, sizeof(temp));
            }
            if (strchr(URL_buf, '\n') != NULL)
            {
                len = strlen(URL_buf);
                URL_buf[len - 1] = '\0';
            }
            strcpy(current->URL, URL_buf);
            prev = current;
            memset(URL_buf, 0, sizeof(URL_buf));
        }
    }
}
int search(struct node_URL *head)
{
    struct node_URL *current;
    current = head;
    while (current != NULL)
    {
        if (strcmp(current->URL, o_URL) == 0)
        {
            return 1;
        }
        current = current->next;
    }
    return 0;
}
void redir(int newfd)
{

    if (search(head) == 1)
    {
        /*         sprintf(redir_doc, "HTTP/1.1 307 Temporary Redirect\nContent-Length: {}\n<head>\n  <meta http-equiv=\"Refresh\" content=\"0; URL=%s/\">\n</head>", "warning.or.kr");
        send(newfd, buf, strlen(buf), 0);
        close(newfd);
        exit(0); */

        strcpy(o_URL, "warning.or.kr");
        strcpy(o_port, "80");
        strcpy(o_object, "http://warning.or.kr/");
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

int fr_ser(int sersock, int newfd, int len)
{
    int t;
    while (1)
    {
        t = recv(sersock, d_buf, len, 0);
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
int recv_client(int newfd)
{
    int status;
    char temp_buf[MAXDATASIZE];
    while ((status = recv(newfd, temp_buf, MAXDATASIZE, 0)) > 0)
    {
        temp_buf[status] = '\0';
        int t = strlen(temp_buf);
        if ((temp_buf[0] == '\n' || temp_buf[0] == '\r') || (temp_buf[t - 1] == '\n' && temp_buf[t - 3] == '\n') || (temp_buf[t - 1] == '\n' && temp_buf[t - 2] == '\n'))
        {
            sprintf(buf, "%s%s", buf, temp_buf);
            break;
        }
        sprintf(buf, "%s%s", buf, temp_buf);
    }
    return strlen(buf);
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
        case 2:
            strcpy(temp, pch);
            strcpy(o_port, temp);
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
    char temp[MAXDATASIZE];
    i++;
    while (pch != NULL)
    {
        switch (i)
        {
        case 1:
            strcpy(temp, pch);
            //fprintf(stdout, "%s", temp);
            strcpy(o_method, temp);
            break;
        case 2:
            strcpy(temp, pch);
            //fprintf(stdout, "%s", temp); //2
            strcpy(o_object, temp);
            //fflush(stdout);
            break;
        default:
            if (strcmp("Host:", pch) == 0)
            {
                pch = strtok(NULL, " \r\n");
                strcpy(temp, pch);
                //fprintf(stdout, "%s", temp); //5
                strcpy(o_URL, temp);
                //fflush(stdout);
            }
        }
        pch = strtok(NULL, " \r\n");
        i++;
    }
};
void Find_HTTP(int newfd)
{
    char http_400[] = "HTTP/1.0 400 Bad Request\r\nconnection: close\r\ncontent-length: 0\r\n";
    char temp[MAXDATASIZE];
    if (strncmp(o_object, "http://", 7) == 0)
    {
        //400
        int len=strlen(http_400);
        sendall(newfd, http_400, &len);
        exit(1); 
    }
}

int main(int argc, char const *argv[])
{
    struct sockaddr_storage their_addr;
    socklen_t addr_size;
    struct addrinfo *res;
    struct addrinfo hints;
    int status;
    int sockfd, newfd;
    int sersock;

    int p[2];

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;
    //setvbuf(stdin, NULL, _IONBF, 0);
    int pid;
    if (argc != 2)
    {
        printf("usage: add a port");
        return -1;
    }
    if ((status = getaddrinfo(NULL, argv[1], &hints, &res)) != 0)
    {
        //fprintf(stderr, "getaddinfo: %s\n", gai_strerror(status));
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
    cr_link_list();

    while (1)
    {
        newfd = accept(sockfd, (struct sockaddr *)&their_addr, &addr_size);
        if (fork() == 0)
        {
            continue;
        }
        /* stat: */
        if (recv_client(newfd) > 0)
        {
            char pch[MAXDATASIZE];
            strcpy(pch, buf);
            segment_h(pch);
            //Find_HTTP(newfd);
            segment(o_URL);
            //TODO redirction

            redir(newfd);
            /* char redir_doc[MAXDATASIZE];
            if (search(head) == 1)
            {
                int len0 = strlen("\n<head>\n  <meta http-equiv=\"Refresh\" content=\"0; URL=warning.or.kr/\">\n</head>");
                sprintf(redir_doc, "HTTP/1.1 307 Temporary Redirect\nContent-Length: %d\n<head>\n  <meta http-equiv=\"Refresh\" content=\"0; URL=%s/\">\n</head>", len0, "warning.or.kr");
                int len1 = strlen(redir_doc);
                sendall(newfd, redir_doc, &len1);
                goto stat;
            } */

            //TODO
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
            sprintf(buf, "%s %s HTTP/1.0\r\nHost: %s\r\n\r\n", "GET", o_object, o_URL);

            send(sersock, buf, strlen(buf), 0);
            memset(buf, 0, strlen(buf));

            int t = 0;
            int s = 0;

            while (1)
            {
                if (s != -1)
                {
                    t = fr_ser(sersock, newfd, MAXDATASIZE);
                }
                if ((t == 0) || (t == -1))
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