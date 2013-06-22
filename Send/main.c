
/* include file for standard library for C */
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

/* define the Parameter */
#define DEVICE_NAME "/dev/fb0"

int main(int argc, char **argv){

    /*check the augment */
    if(argc < 3){
	fprintf(stderr, "usage : ./a.out <hostname> <port>\n");
	exit(1);
    }

    /* open network socket */
    int port = atoi(argv[2]);
    int s;
    struct hostent *host;
    struct sockaddr_in me;
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

    /* FrameBuffer */
    int fd, screensize;

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
    int xres,yres,bpp,line_len;
    xres = vinfo.xres;  yres = vinfo.yres;  bpp = vinfo.bits_per_pixel; 
    line_len = finfo.line_length;
printf("%d(pixel)x%d(line), %d(bit per pixel), %d(line length)\n",xres,yres,bpp,line_len);
    screensize = xres * yres * bpp;

    /*memory I/O */
    char *fbptr;

    fbptr = (char *)mmap(0,screensize,PROT_READ | PROT_WRITE, MAP,fd,0);
    if((int)fbptr == -1){
	printf(stderr,"cannot get framebuffer\n");
	exit(1);
    }
    
printf("the frame buffer device was mapped\n");
    munmap(fbptr,screensize);
    close(fd);


    /* sending a packet on UDP */
    sendto(s, "Hello", 5, 0, (struct sockaddr *)&me,sizeof(me));

    /* close the filediscriptor of socket */
    close(s);


    return 0;
}

