#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

/* include file for socket communication */
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

/* include file for FrameBuffer */
#include <linux/fb.h>
#include <linux/fs.h>
#include <sys/mman.h>

#define DEVICE_NAME "/dev/fb0"

int main(int argc, char **argv){
    if(argc < 3){
	fprintf(stderr, "usage : ./a.out <hostname> <port>\n");
	exit(1);
    }
    int port = atoi(argv[2]);

    struct hostent *host;
    struct sockaddr_in me;
    int s, fd;
    char buf[512];
    host = gethostbyname(argv[1]);
    s = socket(AF_INET, SOCK_DGRAM,0);
    
    bzero((char *)&me,sizeof(me));
    me.sin_family = AF_INET;
    me.sin_port = htons(port);
    bcopy(host->h_addr,(char *)&me.sin_addr, host->h_length);

    if(connect(s,(struct sockaddr *)&me, sizeof(me)) < 0){
	fprintf(stderr, "cannot connect\n");
	exit(1);
    }

    fd = open(DEVICE_NAME, O_RDWR);
    if(fd!){
	printf(stderr, "cannot open the FrameBuffer '/dev/fb0' \n");
	exit(1);
    }

    struct fb_var_screeninfo vinfo;
    struct fb_fix_screeninfo finfo;

    if(ioctl(fd,FBIOGET_FSCREENINFO, &finfo)){
	printf(stderr, "cannot \n");
	exit(1);
    }
    if(ioctl(fd,FBIOGET_VSCREENINFO, &vinfo)){
	printf(stderr, "cannot \n");
	exit(1);
    }

    
    sendto(s, "Hello", 5, 0, (struct sockaddr *)&me,sizeof(me));

    close(s);

    return 0;
}

