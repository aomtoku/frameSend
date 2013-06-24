/* incldue file for Standard Linbrary for C */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/* include file for socket communication */
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

/* include file for FrameBuffer */
#include <linux/fb.h>
#include <linux/fs.h>
#include <sys/mman.h>

/* define the Parameter */
#define DEVICE_NAME "/dev/fb0"


int main(int argc, char **argv)
{
    /* Check the Augments*/
    if(argc < 2){
	fprintf(stderr,"usage : ./a.out <port>");
	exit(1);
    }

    /* open network socket for UDP */
     int sock;
     struct sockaddr_in addr;
     int port = atoi(argv[1]);

     sock = socket(AF_INET, SOCK_DGRAM, 0);
     
     addr.sin_family = AF_INET;
     addr.sin_port = htons(port);
     addr.sin_addr.s_addr = INADDR_ANY;
     
     bind(sock, (struct sockaddr *)&addr, sizeof(addr));

     /* Open a DeviceFile of FrameBuffer */
     int fd, screensize;
     fd = open(DEVICE_NAME, O_RDWR);
     if(!fd){
	 fprintf(stderr,"cannot open the FrameBuffer '/dev/fb0'\n");
	 exit(1);
     }

     struct fb_var_screeninfo vinfo;
     struct fb_fix_screeninfo finfo;

     if(ioctl(fd,FBIGET_FSCREENINFO, &finfo)){
	 fprintf(stderr, "cannot open fix info\n");
	 exit(1);
     }
     if(ioctl(fd,FBIGET_VSCREENINFO, &vinfo)){
	 fprintf(stderr, "cannot open variable info\n");
	 exit(1);
     }
     
     int xres,yres,bpp,line_len;
     xres = vinfo.xres; yres = vinfo.yres; bpp = vinfo.bits_per_pixel;
     line_len = finfo.line_length;

     screensize = xres * yres * bpp / 8;

     /* Handler if socket get a packet, it will be mapped on memory */ 
     char *buf;
     long int location = 0;

     memset(buf, 0, sizeof(buf));

     buf = (char *)mmap(0,screensize,PROT_READ | PROT_WRITE,MAP_SHARED,sock,0);
     if((int)buf == -1){
	 fprintf(stderr, "cannot get framebuffer");

     recvfrom(sock, *(unsigned int)(buf+location), sizeof(unsigned int), 0,&addr, );
     
     printf("%s\n", buf);
     
     close(sock);
     
     return 0;
}

