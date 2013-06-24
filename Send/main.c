
/* include file for standard library for C */
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>

/* include file for socket communication */
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

/* include file for FrameBuffer */
#include <sys/ioctl.h>
#include <linux/fb.h>
#include <linux/fs.h>
#include <sys/mman.h>

/* define the Parameter */
#define DEVICE_NAME "/dev/fb0"

struct packet{
    int xres_screen;
    int yres_screen;
    unsigned int color;
};

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
    if(!fd){
	fprintf(stderr, "cannot open the FrameBuffer '/dev/fb0' \n");
	exit(1);
    }

    struct fb_var_screeninfo vinfo;
    struct fb_fix_screeninfo finfo;

    if(ioctl(fd,FBIOGET_FSCREENINFO, &finfo)){
	fprintf(stderr, "cannot fix info\n");
	exit(1);
    }
    if(ioctl(fd,FBIOGET_VSCREENINFO, &vinfo)){
	fprintf(stderr, "cannot variable info\n");
	exit(1);
    }
    int xres,yres,bpp,line_len;
    xres = vinfo.xres;  yres = vinfo.yres;  bpp = vinfo.bits_per_pixel; 
    line_len = finfo.line_length;
printf("%d(pixel)x%d(line), %d(bit per pixel), %d(line length)\n",xres,yres,bpp,line_len);
    screensize = xres * yres * bpp/8;

    /*memory I/O */
    char *fbptr;
    long int location;
    int x,y = 0;

    fbptr = (char *)mmap(0,screensize,PROT_READ | PROT_WRITE, MAP_SHARED,fd,0);
    if((int)fbptr == -1){
	fprintf(stderr,"cannot get framebuffer\n");
	exit(1);
    }
    
printf("the frame buffer device was mapped\n");
    struct packet packet_udp;
    
    for(y=0;y<yres;y++){
	for(x=0;x<xres;x++){
	    location = ((x+vinfo.xoffset)*bpp/8) + (y+vinfo.yoffset)* line_len;
	    //printf("pointer %p,and the value is %x\n",(unsigned int *)(fbptr+location),*(unsigned int *)(fbptr+location));
	    packet_udp.xres_screen = x; 
	    packet_udp.yres_screen = y; 
	    packet_udp.color = *(unsigned int *)(fbptr+location); 
//printf("x:%d x:%d, y:%d y:%d\n",x,packet_udp.xres_screen,y,packet_udp.yres_screen);
	    //sendto(s, (unsigned int *)(fbptr+location), sizeof(unsigned int *), 0, (struct sockaddr *)&me,sizeof(me));
	    sendto(s, &packet_udp, sizeof(struct packet), 0, (struct sockaddr *)&me,sizeof(me));
	}
    }

    /* sending a packet on UDP */
    //sendto(s, (unsigned int *)(fbptr+location), sizeof(unsigned int *), 0, (struct sockaddr *)&me,sizeof(me));

printf("%d at %s\n",__LINE__,__FILE__);
    munmap(fbptr,screensize);
    /* close the filediscriptor of socket */
    close(s);
    close(fd);

    return 0;
}
