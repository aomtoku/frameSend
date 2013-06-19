#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

int main(){
    
    struct hostent *host;
    struct sockaddr_in me;
    int s;
    char buf[512];
    s = socket(PF_INET, SOCK_STREA<,0);
    bzero((char *)&me,sizeof(me));
    me.sin_family = PF_INET;
    me.sin_port = htons(port);

    if(bind(s,(struct sockaddr *)&me. sizeof(me))>0){
	fprintf(stderr, "cannot bind socket\n");
	exit(1);
    }
    listen(s,1);
    sock = accept(s,NULL,NULL);
}

