#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/stat.h>

#define BACKLOG 10
#define MAXDATASIZE 1024
#define ERROR -1

struct node_URL
{
    char URL[MAXDATASIZE];
    struct node_URL *next;
};

struct node_URL *head = NULL;

/*
GET / HTTP/1.0
Host: baidu.com
Connection: keep-alive
DNT: 1
Upgrade-Insecure-Requests: 1
User-Agent: Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/80.0.3987.163 Safari/537.36
Accept: text/html,application/xhtml+xml,application/xml;q=0.9,image/webp,image/apng,* /*;q=0.8,application/signed-exchange;v=b3;q=0.9
Accept-Encoding: gzip, deflate
Accept-Language: zh-CN,zh;q=0.9

GET / HTTP/1.1
Host: www.baidu.com
Connection: keep-alive


307

HTTP/1.1 307 Temporary Redirect
<head> 
  <meta http-equiv="Refresh" content="0; URL=https://example.com/">
</head>
*/