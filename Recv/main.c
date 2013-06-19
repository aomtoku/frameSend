#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

int main(int argc, char **argv)
{
    if(argc < 2){
	fprintf(stderr,"usage : ./a.out <port>");
	exit(1);
    }
     int sock;
     struct sockaddr_in addr;

     char buf[2048];
     int port = atoi(argv[1]);

     sock = socket(AF_INET, SOCK_DGRAM, 0);
     
     addr.sin_family = AF_INET;
     addr.sin_port = htons(port);
     addr.sin_addr.s_addr = INADDR_ANY;
     
     bind(sock, (struct sockaddr *)&addr, sizeof(addr));
     
     memset(buf, 0, sizeof(buf));
     recv(sock, buf, sizeof(buf), 0);
     
     printf("%s\n", buf);
     
     close(sock);
     
     return 0;
}

