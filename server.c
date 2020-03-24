#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>

int status;
struct addrinfo hints;
struct addrinfo *servinfo;

memset(&hints,0,sizeof (hints));
hints.ai_family = AF_UNSPEC;
hints.ai_socktype=SOCK_STREAM;

getaddrinfo("dbd039.cn","http",&hints,&servinfo);
