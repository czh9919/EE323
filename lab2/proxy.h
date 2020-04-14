#include <stdio.h>
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

struct HTTP_Header
{
    char method[100];// GET
    char obect[MAXDATASIZE];
    char version[100];// HTTP/1.1
    char host[100];// Host:
    char URL[MAXDATASIZE];
    char *(i_Data[]);
/*     char* connect;
    char* status; */
};

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

Get / HTTP/1.1
Host: www.baidu.com
Connection: keep-alive


*/