/*
 * X bitmap to Mgr icon conversion.
 * This is an example of code which writes an icon file
 * when compiled and run, the ascii text from the .xbm file
 * being edited into the initialization part of ic, below.
 * Auther: Vincent Broman, broman@nosc.mil, dec 1993.
 */
#define ICONNAME "tex-vf"
#include <mgr/mgr.h>
static unsigned char ic[] = {
   0xf0, 0xff, 0x3f, 0x00, 0xf0, 0xff, 0x7f, 0x00, 0xf0, 0xff, 0xbf, 0x00,
   0xf0, 0xff, 0x3f, 0x01, 0xf0, 0xff, 0x3f, 0x02, 0xf0, 0xff, 0x3f, 0x04,
   0xf0, 0xff, 0x3f, 0x08, 0xf0, 0xff, 0xff, 0x1f, 0xf0, 0xff, 0xff, 0x1f,
   0xf0, 0xff, 0xff, 0x1f, 0xf0, 0xff, 0xff, 0x1f, 0xf0, 0xff, 0xff, 0x1f,
   0xf0, 0xff, 0xff, 0x1f, 0xf0, 0xff, 0xff, 0x1f, 0xf0, 0xff, 0x0f, 0x1f,
   0xf0, 0xff, 0x67, 0x1f, 0xf0, 0xff, 0xe7, 0x1f, 0xf0, 0x30, 0x82, 0x1f,
   0xf0, 0x79, 0xe7, 0x1f, 0xf0, 0x73, 0xe7, 0x1f, 0xf0, 0xb3, 0xe7, 0x1f,
   0xf0, 0xa7, 0xe7, 0x1f, 0xf0, 0xc7, 0xe7, 0x1f, 0xf0, 0xcf, 0xe7, 0x1f,
   0xf0, 0xef, 0xc3, 0x1f, 0xf0, 0xff, 0xff, 0x1f, 0xf0, 0xff, 0xff, 0x1f,
   0xf0, 0xff, 0xff, 0x1f, 0xf0, 0xff, 0xff, 0x1f, 0xf0, 0xff, 0xff, 0x1f,
   0xf0, 0xff, 0xff, 0x1f, 0xe0, 0xff, 0xff, 0x1f};

unsigned char
flip(unsigned char c) {
    int i, j;
    unsigned char oc = 0;

    for( i=128,j=1; i; i>>=1,j<<=1)
      if( i & c)
	oc |= j;
    return oc;
}

void
main(void){
    int fd;
    struct b_header hdr;
    unsigned int i;
    unsigned char c;

    B_PUTHDR8(&hdr,32,32,1);
    fd=creat(ICONNAME,0644);
    write(fd,&hdr,sizeof(hdr));
    for(i=0;i<sizeof(ic);i+=1) {
      c = flip(ic[i]);
      write(fd,&c,1);
    }
    close(fd);
    exit(fd < 0);
}
