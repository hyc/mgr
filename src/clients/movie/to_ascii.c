/*{{{}}}*/
/*{{{  Notes*/
/* read op file (binary format) convert to ascii */
/*}}}  */
/*{{{  #includes*/
#include <mgr/bitblit.h>
#include <mgr/share.h>

#include "getshort.h"
/*}}}  */
/*{{{  #defines*/
#define A(x)		args[x]		/* short hand */
/*}}}  */

/*{{{  variables*/
unsigned short args[10];
char line[80];
/*}}}  */

/*{{{  print_data -- print the data in ascii*/
static void print_data(int w, int h, int out)
{
  int bytes = bit_size(w,h,1);
  register int i,c;

  printf(". ");
  for (i=1; i<=bytes && (c=getchar())!=EOF; i++)
  if (out) printf
  (
    "%c%c%s",
    "0123456789ABCDEF"[(c>>4)&0xF],
    "0123456789ABCDEF"[c&0xF],
    i%30==0 && i<bytes ? "\n. " : ""
  );
  printf("\n");
}
/*}}}  */

/*{{{  main*/
int main(int argc,char *argv[])
{
  int c;
  int time = 0;
  int no_data=0;

  if (argc>1) no_data++;

  while((c=getshort())!=EOF)
  {
    /*{{{  don't flood with time messages*/
    if (time && (c&TYPE_MASK)!=T_TIME) fputs(line,stdout);
    time=0;
    /*}}}  */

    switch (c&TYPE_MASK)
    {
      /*{{{  T_BLIT*/
      case T_BLIT:
      getnshort(args,8);
      printf
      (
        "B %d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\n",
        args[0],
        args[2],
        args[3],
        args[4],
        args[5],
        OPCODE(c),
        args[1],
        args[6],
        args[7]
      );
      break;
      /*}}}  */
      /*{{{  T_WRITE*/
      case T_WRITE:
      getnshort(args,5);
      printf
      (
        "W %d\t%d\t%d\t%d\t%d\t%d\n",
        args[0],
        args[1],
        args[2],
        args[3],
        args[4],
        OPCODE(c)
      );
      break;
      /*}}}  */
      /*{{{  T_SCREEN*/
      case T_SCREEN:
      getnshort(args,4);
      printf("S %d\t%d\t%d\t%d\n",A(0),A(1),A(2),A(3)?1:0);
      if (A(3)) print_data(A(1),A(2),!no_data);
      break;
      /*}}}  */
      /*{{{  T_NOP*/
      case T_NOP:
      c &= 0xF;		/* # of bytes to skip */
      while(c-->0) getchar();
      break;
      /*}}}  */
      /*{{{  T_LINE*/
      case T_LINE:
      getnshort(args,5);
      printf("L %d\t%d\t%d\t%d\t%d\t%d\n",
      A(0),A(1),A(2),A(3),A(4),c&0xf);
      break;
      /*}}}  */
      /*{{{  T_DATA*/
      case T_DATA:
      getnshort(args,4);
      printf("D %d\t%d\t%d\t%d\n",A(0),A(1),A(2),A(3)?1:0);
      if (A(3)) print_data(A(1),A(2),!no_data);
      break;
      /*}}}  */
      /*{{{  T_TIME*/
      case T_TIME:
      getnshort(args,2);
      sprintf(line,"T %d.%d\n",(args[0]<<16|args[1])/100,(args[0]<<16|args[1])%100);
      time++;
      break;
      /*}}}  */
      /*{{{  T_POINT*/
      case T_POINT:
      getnshort(args,3);
      printf("P %d\t%d\t%d\t%d\n",args[0],args[1],args[2],OPCODE(c));
      break;
      /*}}}  */
      /*{{{  T_KILL*/
      case T_KILL:
      getnshort(args,1);
      printf("K %d\n",args[0]);
      break;
      /*}}}  */
      /*{{{  T_MARK*/
      case T_MARK:
      getnshort(args,1);
      printf("M %d\n",args[0]);
      break;
      /*}}}  */
      /*{{{  T_BYTESCROLL*/
      case T_BYTESCROLL:
      getnshort(args,6);
      printf("C %d\t%d\t%d\t%d\t%d\t%d\n",args[0],args[1],args[2],args[3],args[5],args[5]);
      break;
      /*}}}  */
      /*{{{  default*/
      default:
      fprintf(stderr,"OOps, got 0x%x\n",c);
      break;
      /*}}}  */
    }
  }
  return 0;
}
/*}}}  */
