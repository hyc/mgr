#define DATA unsigned char

#include <bitblit.h>

extern DATA *graph_mem;

#define LOGBITS 3
#define BITS (~(~(unsigned)0<<LOGBITS))

#define bit_linesize(wide,depth) (((depth*wide+BITS)&~BITS)>>3)
#define BIT_LINE(x) ((((x)->primary->depth*(x)->primary->wide+BITS)&~BITS)>>LOGBITS)

void setplane(int plane);
void setmapmask(int mask);
void write_mode(int mode);
/*{{{}}}*/
