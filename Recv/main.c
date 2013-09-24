/*****************************************************
 *						     *
 *						     *
 *						     *
 *
 *  ### CHECK FIREWALLs on your system               *
 ****************************************************/		


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

#define DATA_SIZE 1280
#define RGB_BYTE 4
#define BIT 8


struct packet{
    short int xres_screen;
    short int yres_screen;
    unsigned char color[DATA_SIZE];
};


int BindUDPconnect(int sock, struct sockaddr_in addr, struct sockaddr_in recv, int port){
     sock = socket(AF_INET, SOCK_DGRAM, 0);
     
     addr.sin_family = AF_INET;
     addr.sin_port = htons(port);
     addr.sin_addr.s_addr = INADDR_ANY;
     
     if(bind(sock, (struct sockaddr *)&addr, sizeof(addr)) == -1){
	 fprintf(stderr,"cannot bind\n");
	 exit(1);
     }
    return sock;
}

int OpenFrameBuffer(int fd){
     fd = open(DEVICE_NAME, O_RDWR);
     if(!fd){
	 fprintf(stderr,"cannot open the FrameBuffer '%s'\n",DEVICE_NAME);
	 exit(1);
     }

     return fd;
}

     

/*
char *InitMemoryMap(char *buf,int screensize,int fd){
     buf = (char *)mmap(0, screensize, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
     if((int)buf == -1){
	 fprintf(stderr, "cannot get framebuffer");
	 exit(1);
     }
     
    return buf;
}
*/
void LoopRecvPacket(int sock, struct sockaddr_in recv, char *buf, struct fb_var_screeninfo vinfo, int line_len, int bpp){
     long int location = 0;
     struct packet rec_packet;
     int rec;
     socklen_t sin_size = sizeof(struct sockaddr_in);

     while(1){
	 if((rec = recvfrom(sock, &rec_packet, sizeof(struct packet), 0,(struct sockaddr *)&recv, &sin_size)) == -1){
	     fprintf(stderr, "cannot receive a packet \n");
	     exit(1);
	 }
	 location = ((rec_packet.xres_screen + vinfo.xoffset) * bpp/8) + (rec_packet.yres_screen+vinfo.yoffset)*line_len;

	 int x_pos_cnt;
	 for(x_pos_cnt=0;x_pos_cnt<320;x_pos_cnt++){
#if 0
	     int val;
	     if(rec_packet.xres_screen == 320) {
		 val=0;
		 val = j;
	     } else val = j;
	     location = ((rec_packet.xres_screen+val + vinfo.xoffset)*bpp/8) + (rec_packet.yres_screen+vinfo.yoffset)*line_len;
#else
	     location = ((rec_packet.xres_screen+ x_pos_cnt + vinfo.xoffset)*bpp/8) + (rec_packet.yres_screen+vinfo.yoffset)*line_len;
#endif
	     //printf("ptr=%p *val=%#x &val=%#x \n",rec_packet.color,*(rec_packet.color + j), &rec_packet.color);
	     memcpy(buf+location,(unsigned int *)(rec_packet.color+(x_pos_cnt*RGB_BYTE)),sizeof(unsigned int *));
	 }
	 msync((unsigned int *)(buf+location),sizeof(unsigned int *),MS_ASYNC);
     }
}

int main(int argc, char **argv)
{
    /* Check the Augments*/
    if(argc < 2){
	fprintf(stderr,"usage : ./a.out <port>");
	exit(1);
    }
    /* open network socket for UDP */
     int sock = 0;
     struct sockaddr_in addr;
     struct sockaddr_in recv;
     int port = atoi(argv[1]);


     sock = BindUDPconnect(sock,addr,recv,port);
     
     /* Open a DeviceFile of FrameBuffer */
     int fd = 0; 
     int screensize;
     fd = OpenFrameBuffer(fd);
     
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
     
     char *buf =0;
     //buf = InitMemeoryMap(buf,screensize,fd);

     buf = (char *)mmap(0, screensize, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
     if((int)buf == -1){
	 fprintf(stderr, "cannot get framebuffer");
	 exit(1);
     }
     
     /* Loop for Recvfrom SOCKET UDP */
     LoopRecvPacket(sock, recv, buf, vinfo, line_len, bpp);
     
     munmap(buf,screensize);

     close(fd);
     close(sock);

     return 0;
}
