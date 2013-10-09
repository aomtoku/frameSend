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

/* DEFINE the Parameter */
#define DEVICE_NAME "/dev/fb0"


#define BIT 8
#define YUVMODE
#define DATA_YUV

#ifdef DATA_YUV

#define DATA_SIZE 1280
#define PIXEL_PER_PACKET 640
#define RGB_BYTE 2
#define DISPLAY_XRES 1280
#define DISPLAY_YRES 1440

#else

#define DATA_SIZE 960
#define PIXEL_PER_PACKET 320
#define RGB_BYTE 3
#define DISPLAY_XRES 1280
#define DISPLAY_YRES 720


#endif

/*
struct packet{
    unsigned short int yres_screen;
    unsigned short int xres_screen;
    unsigned short int color[DATA_SIZE/2];
};*/

struct packet{
    unsigned short int yres_screen;
    unsigned short int xres_screen;
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
#ifdef YUVMODE
struct RGB{
    char B;
    char G;
    char R;
};

int yuv2rgb(char *rgb_color, struct packet *yuv_packet, int packet_cnt){
    unsigned short int yuv_y = *(yuv_packet->color+(packet_cnt*RGB_BYTE));
    unsigned short int yuv_cb,yuv_cr;
    if((packet_cnt%2) == 1){
	yuv_cb = *(yuv_packet->color+(packet_cnt*RGB_BYTE)+1);
	yuv_cr = *(yuv_packet->color+((packet_cnt-1)*RGB_BYTE)+1);
    } else {
	yuv_cr = *(yuv_packet->color+(packet_cnt*RGB_BYTE)+1 );
	yuv_cb = *(yuv_packet->color+((packet_cnt-1)*RGB_BYTE)+1);
    }
    int over_value = 0;
 
    //printf("yuv = %u %u %u\n",yuv_y, yuv_cb, yuv_cr);
    short int rgb_r,rgb_g,rgb_b;
    //if((yuv_y < hoge) & (yuv_cr < hoge) & (yuv_cb < hoge)){
	rgb_r = 1.164 * (yuv_y - 16) + (1.696 * (yuv_cr - 128));
	rgb_g = 1.164 * (yuv_y - 16) - (0.391 * (yuv_cb - 128)) - (0.813 * (yuv_cr - 128));
	rgb_b = 1.164 * (yuv_y - 16) + (2.018 * (yuv_cb - 128));
   /* } else {
	over_value = 1;
    }*/
    //printf("after rgb = %u %u %u\n",rgb_r, rgb_g, rgb_b);
    rgb_color[0] = rgb_r;
    rgb_color[1] = rgb_g;
    rgb_color[2] = rgb_b; 
    //printf("%d %d %d \n",rgb_color[0],rgb_color[1],rgb_color[2] );
    return over_value;
}
#endif

void LoopRecvPacket(int sock, struct sockaddr_in recv, char *buf, struct fb_var_screeninfo vinfo, int line_len, int bpp){
     long int location = 0;
     struct packet rec_packet;
     int rec;
     socklen_t sin_size = sizeof(struct sockaddr_in);
#ifdef YUVMODE
     char rgb_color;
     rgb_color = (char *)malloc(sizeof(char)*3);

     int CbCr_flag=1;
#endif

     while(1){
	 if((rec = recvfrom(sock, &rec_packet, sizeof(struct packet), 0,(struct sockaddr *)&recv, &sin_size)) == -1){
	     fprintf(stderr, "cannot receive a packet \n");
	     exit(1);
	 }
//printf("X pos = %d, Y = %d\n",rec_packet.xres_screen,rec_packet.yres_screen);
	 location = ((rec_packet.xres_screen + vinfo.xoffset) * bpp/8) + (rec_packet.yres_screen+vinfo.yoffset)*line_len;

	 //if((rec_packet.xres_screen < (DISPLAY_XRES - PIXEL_PER_PACKET)) & (rec_packet.yres_screen < DISPLAY_YRES)){
	     int x_pos_cnt;
	     int xpos,ypos;
	     for(x_pos_cnt=0;x_pos_cnt<PIXEL_PER_PACKET;x_pos_cnt++){
		 if((rec_packet.xres_screen + x_pos_cnt) >= DISPLAY_XRES){
		     xpos = rec_packet.xres_screen + x_pos_cnt - DISPLAY_XRES;
		     ypos = rec_packet.yres_screen + 1;
		     /*if((ypos & 1) == 1)
			 continue;
			 ypos = ypos/2;*/
		     location = ((xpos + vinfo.xoffset)*bpp/8) + (ypos+vinfo.yoffset)*line_len;
		     memcpy(buf+location+1,(unsigned int *)(rec_packet.color+(x_pos_cnt*RGB_BYTE)),RGB_BYTE);
		 } else {
		     xpos = rec_packet.xres_screen + x_pos_cnt;
		     ypos = rec_packet.yres_screen;
		     /*if((ypos & 1) == 1)
			 continue;
			 ypos = ypos/2;*/
		     location = ((xpos + vinfo.xoffset)*bpp/8) + (ypos+vinfo.yoffset)*line_len;
#ifdef YUVMODE
		     //struct RGB recRGB,dispRGB;
		     //memcpy(&recRGB,(unsigned int *)(rec_packet.color+(x_pos_cnt*RGB_BYTE)),RGB_BYTE);
		     //memcpy(&rgb_color,(unsigned int *)(rec_packet.color+(x_pos_cnt*RGB_BYTE)),RGB_BYTE);
//printf("YUV = %u %u %u\n",recRGB.R,recRGB.G,recRGB.B);		     
		     //dispRGB = YUV2RGB(recRGB,dispRGB,CbCr_flag);
		     //printf("X = %d, %d, %d\n",*(rec_packet.color+(x_pos_cnt*RGB_BYTE)), *(rec_packet.color+(x_pos_cnt*RGB_BYTE)+1) ,*(rec_packet.color+(x_pos_cnt*RGB_BYTE)+2));
		     int check_yuv;
		     if(check_yuv = yuv2rgb(&rgb_color, &rec_packet, x_pos_cnt) == 1){
			 printf("error\n");
		     }

//printf("RGB = %u %u %u\n",dispRGB.R,dispRGB.G,dispRGB.B);		     
		     memcpy(buf+location,&rgb_color,3);
#else
		     //printf("X = %d, %d\n",*(rec_packet.color+(x_pos_cnt*RGB_BYTE)), *(rec_packet.color+(x_pos_cnt*RGB_BYTE)+1) );
//printf("X[%d] = %u, %d\n",x_pos_cnt,rec_packet.color[x_pos_cnt], *(rec_packet.color+(x_pos_cnt*RGB_BYTE)+1) );
		     memcpy(buf+location+1,(unsigned int *)(rec_packet.color+(x_pos_cnt*RGB_BYTE)),RGB_BYTE);
#endif
		 }
		 //printf("%d %d \n",xpos,ypos);
		 //location = ((xpos + vinfo.xoffset)*bpp/8) + (ypos+vinfo.yoffset)*line_len;
		 //printf("ptr=%p *val=%#x &val=%#x \n",rec_packet.color,*(rec_packet.color + j), &rec_packet.color);
		 //memcpy(buf+location,(unsigned int *)(rec_packet.color+(x_pos_cnt*RGB_BYTE)),sizeof(unsigned int *));
	     } 
	/* } else {
printf("error!\n");
	 }*/
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
     printf("RECVFRAM Atlys Ver0.1\n%d(pixel)x%d(line), %d(bit per pixel), %d(line length)\n",xres,yres,bpp,line_len);
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
