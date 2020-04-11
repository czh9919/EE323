#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
 
static usage(const char* proc)
{
	printf("%s -h [ip] -p [port]\n", proc);
}
 
int main(int argc, char** argv)
{
	if(argc < 5)
	{
		usage(argv[0]);
		exit(1);
	}
    int ch0;
    char ip[16], port[16];
    while ((ch0 = getopt(argc,argv,"h:p:"))!=-1) 
    {
       switch(ch0) 
       {  
             case 'h':
                       strcpy(ip, optarg);
                       //printf("option h:'%s'\n",optarg);  
                       break;  
             case 'p':  
                       strcpy(port, optarg);
                       //printf("option p:'%s'\n",optarg);      
                       break;                       
             default:                              
                       printf("other option :c\n",ch0);                
        }        
    }
    
	int sock = socket(AF_INET, SOCK_STREAM, 0);
	struct sockaddr_in server;
	if(sock < 0)
	{
		perror("socket");
		return 1;
	}
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = inet_addr(ip);
	server.sin_port = htons(atoi(port));
 
	if(connect(sock ,(struct sockaddr*)&server, sizeof(server) ) < 0)
	{
		perror("connect");
		exit(4);
	}
 
	char buf[1024]="hello server";
    char ch;
	write( sock, buf, strlen(buf));
	read(sock, buf, sizeof(buf)-1);
	printf( "%s\n", buf  );
	memset(buf, '\0', sizeof(buf));
	int n = 0, i = 0;
    
    if (!isatty(fileno(stdin)))
    {
       while ((ch = getchar())!= EOF )
       {
            if( ch == '\n'  )
            {
                printf( "%s", buf  );
                write( sock, buf, strlen(buf));
                memset(buf,'\0',sizeof(buf) );
                i = 0;
                printf( "\n" );
                sleep(1);
            }
            else
            {
                buf[i++] = ch;
            }
        }
        close(sock);
        return 0;
    }
	while(1)
	{
		//printf("please input #: ");
		fflush(stdout);
		ssize_t sread = read(0, buf, sizeof(buf)-1);
        //fgets(buf, 1024 , stdin);
        //ssize_t sread = strlen(buf);
		//printf("%d\n", sread);
		if(sread > 0)
		{
			//printf( "%d\n", buf[0] );
			if( buf[0] == '\n'  )
            {
				n = n + 1;
				if( n == 2 )
				{
					break;
				}
            }
			else
			{
				n = 0;
			}
			buf[sread-1] = 0;
			write(sock, buf, strlen(buf));
			printf("server $ %s\n", buf);
			///printf( "%d\n", buf[0] );
			//read(sock, buf, sizeof(buf) - 1);
		}	
	} 
	close(sock);
	return 0;
}
