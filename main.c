#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

int main(int argc, char **argv){
    int port;
    struct hostent *host;
    struct sockaddr_in me;
    int s;
    char buf[512];
    s = socket(AF_INET, SOCK_DGRAM,0);
    
    bzero((char *)&me,sizeof(me));
    me.sin_family = AF_INET;
    me.sin_port = htons(port);

    if(bind(s,(struct sockaddr *)&me, sizeof(me)) < 0){
	fprintf(stderr, "cannot bind socket\n");
	exit(1);
    }
    
    sendto(s, "Hello", 5, 0, (struct sockaddr *)&me,sizeof(me));

    close(s);

    return 0;
}

