/*****************************************************
 *                                                   *
 *      This program is test for FrameBuffer         *
 *                                                   *
 *                                                   *
 *                                                   *
 *                                                   *
 *                                                   *
 *************************************************::*/

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>

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
#define DIV_BYTE 8

#define X_PIXEL_MAX 1600
#define Y_LINE_MAX  1200

#define BORDER1 400
#define BORDER2 800

#define COLOR_RED    0x7FE00000
#define COLOR_GREEN  0x003FFC00
#define COLOR_BLUE   0x000003FF
#define COLOR_WHITE  0xffffffaa
#define COLOR_BLACK  0x0000
#define COLOR_YELLOW 0xffe0

int main(int argc, char **argv){

    /*check the augment */
    /*if(argc < 3){
	fprintf(stderr, "usage : ./a.out <hostname> <port>\n");
	exit(1);
    }*/

    /* open network socket */
    /*int port = atoi(argv[2]);
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
    }*/

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
	fprintf(stderr, "cannot open fix info\n");
	exit(1);
    }
    if(ioctl(fd,FBIOGET_VSCREENINFO, &vinfo)){
	fprintf(stderr, "cannot open var info\n");
	exit(1);
    }
    int xres,yres,bpp,line_len;
    xres = vinfo.xres;  yres = vinfo.yres;  bpp = vinfo.bits_per_pixel; 
    line_len = finfo.line_length;
printf("%d(pixel)x%d(line), %d(bit per pixel), %d(line length)\n",xres,yres,bpp,line_len);
    screensize = xres * yres * bpp/8;

    /*memory I/O */
    char *fbptr;
    int x,y;
    unsigned int tcolor ;//32bit color
    long int location;

    fbptr = (char *)mmap(0,screensize,PROT_READ | PROT_WRITE, MAP_SHARED,fd,0);
    if((int)fbptr == -1){
	fprintf(stderr,"cannot get framebuffer\n");
	exit(1);
    }
    
printf("the frame buffer device was mapped\n");
    /* 表示 */
    for ( y = 0; y < Y_LINE_MAX; y++) {
	/* 色決定a*/
	tcolor = COLOR_BLUE;
	if ( y > BORDER2 ) {
	    tcolor = COLOR_WHITE;
	} else {
	    if ( y > BORDER1 ) {
		tcolor = COLOR_GREEN ;
	    }
	}
	/* １ライン処理 */
	for ( x = 0; x < X_PIXEL_MAX; x++ ) {
	    /* 格納位置計算 */
	    location = ((x+vinfo.xoffset) * bpp / DIV_BYTE) + (y+vinfo.yoffset) * line_len;
	    /* 着色 */
	    *((unsigned int *)(fbptr + location)) = tcolor;
	}
    }
    munmap(fbptr,screensize);
printf("the frame buffer device was unmapped\n");
    close(fd);


    /* sending a packet on UDP */
    //sendto(s, "Hello", 5, 0, (struct sockaddr *)&me,sizeof(me));

    /* close the filediscriptor of socket */
    //close(s);


    return 0;
}

