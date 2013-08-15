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

#define DATA_SIZE 1281
#define BIT 8


struct packet{
    short int xres_screen;
    short int yres_screen;
    char color[DATA_SIZE];
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
     int fd, screensize,rec;
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

     screensize = xres * yres * bpp / BIT;
printf("%d(pixel)x%d(line), %d(bit per pixel), %d(line length)\n",xres,yres,bpp,line_len);
     /* Handler if socket get a packet, it will be mapped on memory */ 
     
     char *buf;
     long int location = 0;
     struct packet rec_packet;

     buf = (char *)mmap(0, screensize, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
     if((int)buf == -1){
	 fprintf(stderr, "cannot get framebuffer");
	 exit(1);
     }
     

     /* Loop for Recvfrom SOCKET UDP */
     while(1){
	 rec = recvfrom(sock, &rec_packet, sizeof(struct packet), 0,(struct sockaddr *)&recv, &sin_size);
	 if(rec < 0){
	     fprintf(stderr, "cannot receive a packet \n");
	     exit(1);
	 }
	 location = ((rec_packet.xres_screen + vinfo.xoffset)*bpp/8) + (rec_packet.yres_screen+vinfo.yoffset)*line_len;

	 //printf("ptr=%p *val=%#x &val=%#x \n",rec_packet.color,*(rec_packet.color + t), &rec_packet.color);
	 int j;
	 for(j=0;j<320;j++){
	     int val;
	     if(rec_packet.xres_screen == 320) {
		 val=0;
		 val = j;
	     } else val = j;
	     location = ((rec_packet.xres_screen+val + vinfo.xoffset)*bpp/8) + (rec_packet.yres_screen+vinfo.yoffset)*line_len;
	     memcpy(buf+location,(unsigned int *)(rec_packet.color+(j*3)),sizeof(unsigned int *));
	 }
	 msync((unsigned int *)(buf+location),sizeof(unsigned int *),MS_ASYNC);
     }

     munmap(buf,screensize);

     close(fd);
     close(sock);

     printf("success\n");
     
     return 0;
}
