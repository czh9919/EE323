#include<stdio.h>
#include<unistd.h>
#include<getopt.h>

#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <arpa/inet.h>

static usage(const char* proc)
{
    printf("%s -p [port]\n", proc);
}

int main(int argc, char *argv[])
{   
    if(argc < 3)
    {
        usage(argv[0]);
        exit(1);
    }
    int sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock < 0)
	{
		perror("----sock----fail\n");
		exit(-1);
	}
    //bind local socket
    int _port = 5711;
	struct sockaddr_in local;
	local.sin_family = AF_INET;
	local.sin_addr.s_addr = INADDR_ANY;
	local.sin_port = htons(atoi(argv[2]));
    struct sockaddr_in client;
	socklen_t len = sizeof(client);
    char buf[1024];
    
	if(bind(sock, (struct sockaddr*)&local, sizeof(local)) != 0)
	{
		perror("---bind---fail\n");
		close(sock);
		exit(-2);
	}
 
	if(listen(sock, 5) != 0)
	{
		perror("----listen----fail\n");
		close(sock);
		exit(-1);
	}
    while(1)
	{
		int new_sock = accept(sock,(struct sockaddr*)&client, &len);
		if(new_sock < 0)
		{
			perror("----accept----fail\n");
			close(sock);
			exit(-5);
		}
		pid_t id = fork();
		if(id == 0)
		{
			
			close(sock);
			while(1)
			{
			    ssize_t s = read(new_sock,buf, sizeof(buf)-1);
			    if(s > 0 )
			    {
			    	buf[s] = 0;
			    	printf("%s\n", buf);
                    //char k[] = "hello server";
                    //int len1 = strlen(buf);
                    //int len2 = strlen(k);
                    //printf( "len1 = %d len2 = %d\n", len1, len2 );
                    if( 0 == strcmp( buf, "hello server" ) )
                    {
                        ////printf("rec rec!!!\n");
                        char rec[] = "hello client";
                        write(new_sock, rec, strlen(rec));
                    }
			    }
				else if( s == 0)
				{
					//printf("client bye. \n");
					break;
				}
				else
                {
					break;
                }
			}
			exit(0);
 
		}
		else if (id > 0)
		{
			close(new_sock); 
			if(fork() > 0)
			{
				exit(0);
			}
		}
	}
    return 0;
}
