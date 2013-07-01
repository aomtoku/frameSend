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

#define VGA_X 640
#define VGA_Y 480

#define DATA_SIZE 1280
#define BIT 8
#define RED_DEC 2145386496


/* Debug Parameter */
//#define DEBUG_RED_DISP
#define NODEBUG

/* Struct for Packet DataGrum */
struct packet{
    short int xres_screen;
    short int yres_screen;
    unsigned char color[DATA_SIZE];
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
    
    /* Check your machine's resolusion and pixel line length(bytes) */
    printf("%d(pixel)x%d(line), %d(bit per pixel), %d(line length)\n",xres,yres,bpp,line_len);
    screensize = xres * yres * bpp/BIT;

    /*memory I/O */
    char *fbptr;
    long int location;
    int x = 0;
    int y = 0;
    int cnt = 0;

    fbptr = (char *)mmap(0,screensize,PROT_READ | PROT_WRITE, MAP_SHARED,fd,0);
    if((int)fbptr == -1){
	fprintf(stderr,"cannot get framebuffer\n");
	exit(1);
    }
printf("the frame buffer device was mapped\n");
    struct packet packet_udp;
    packet_udp.xres_screen = x; 
    packet_udp.yres_screen = y; 
    int ycnt=0;
    int snd;
#ifdef DEBUG_RED_DISP
    unsigned int *num;
    num = (unsigned int  *)malloc(sizeof(unsigned int *));
    *num = RED_DEC;
#endif 
    while(1){
    for(y=0;y<VGA_Y;y++){
	for(x=0;x<VGA_X;x++){
	    location = ((x+vinfo.xoffset)*bpp/8) + (y+vinfo.yoffset)* line_len;
	    if(cnt == 319){
#ifdef DEBUG_RED_DISP
		memcpy(packet_udp.color+(cnt*4), num,sizeof(unsigned int *) );
#else
		memcpy(packet_udp.color+(cnt*4), (char *)(fbptr+location), sizeof(unsigned int *));
#endif
		if(x == VGA_X - 1) {
		    packet_udp.xres_screen = VGA_X/2;
		    packet_udp.yres_screen = y;
		} else {
		    packet_udp.xres_screen = 0; 
		    packet_udp.yres_screen = y;
		}
		
		/* Recvfrom gets a packet for UDP with returning error*/
		do {
		    snd = send(s, &packet_udp, sizeof(struct packet), 0);
		} while( send < 0 && (errno == EAGAIN || errno == EWOULDBLOCK ));

		if(send < 0){
		    fprintf(stderr,"cannot send a packet \n");
		    exit(1);
		}

		cnt = 0;
	    } else {
#ifdef DEBUG_RED_DISP
		memcpy(packet_udp.color+(cnt*4), num,sizeof(unsigned int *) );
#else
		memcpy(packet_udp.color+(cnt*4), fbptr+location, sizeof(unsigned int *));
#endif		
		cnt++;
	    }
	}
	ycnt++;
    }
    }

    munmap(fbptr,screensize);
    
    /* close the filediscriptor of socket */
    close(s);
    close(fd);

    return 0;
}

