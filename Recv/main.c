/* incldue file for Standard Linbrary for C */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>

/* include file for socket communication */
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

/* include file for FrameBuffer */
#include <linux/fb.h>
#include <linux/fs.h>
#include <sys/mman.h>
#include <sys/ioctl.h>

/* define the Parameter */
#define DEVICE_NAME "/dev/fb0"

struct packet{
    int xres_screen;
    int yres_screen;
    unsigned int color;
};


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
     struct sockaddr_in recv;
     int port = atoi(argv[1]);
     socklen_t sin_size;

     sin_size = sizeof(struct sockaddr_in);

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

     if(ioctl(fd,FBIOGET_FSCREENINFO, &finfo)){
	 fprintf(stderr, "cannot open fix info\n");
	 exit(1);
     }
     if(ioctl(fd,FBIOGET_VSCREENINFO, &vinfo)){
	 fprintf(stderr, "cannot open variable info\n");
	 exit(1);
     }
     
     int xres,yres,bpp,line_len;
     xres = vinfo.xres; yres = vinfo.yres; bpp = vinfo.bits_per_pixel;
     line_len = finfo.line_length;

     screensize = xres * yres * bpp / 8;
printf("%d(pixel)x%d(line), %d(bit per pixel), %d(line length)\n",xres,yres,bpp,line_len);
     /* Handler if socket get a packet, it will be mapped on memory */ 
     char *buf;
     long int location = 0;
     //int x=0;int y=0;

     //memset(buf, 0, sizeof(char *));
     struct packet rec_packet;

     buf = (char *)mmap(0, screensize, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
     if((int)buf == -1){
	 fprintf(stderr, "cannot get framebuffer");
	 exit(1);
     }
     while(1){
	 //location = ((x + vinfo.xoffset)*bpp/8) + (y+vinfo.yoffset)*line_len;
	 //recvfrom(sock, (unsigned int *)(buf+location), sizeof(unsigned int *), 0,(struct sockaddr *)&recv, &sin_size);
	 recvfrom(sock, &rec_packet, sizeof(struct packet), 0,(struct sockaddr *)&recv, &sin_size);
//printf("x:%d, y:%d, color:%x\n",rec_packet.xres_screen, rec_packet.yres_screen, rec_packet.color);
	 location = ((rec_packet.xres_screen + vinfo.xoffset)*bpp/8) + (rec_packet.yres_screen+vinfo.yoffset)*line_len;
	 *(unsigned int *)(buf+location) = rec_packet.color;
	 //printf("the value: %x\n",*(unsigned int *)(buf+location));
	 msync((unsigned int *)(buf+location),sizeof(unsigned int *),MS_ASYNC);
	 //printf("%x\n", *(buf+location));
	 /*if(x == xres) {
	     x = 0;
	     if(y==yres){
	     y = 0;
	     } else y ++;
	 } else x++;*/
     }

     munmap(buf,screensize);

     close(fd);
     close(sock);
     
     return 0;
}

