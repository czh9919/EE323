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
