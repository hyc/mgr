/*{{{}}}*/
/*{{{  Notes*/
/* convert ascii back into binary */
/*}}}  */
/*{{{  #includes*/
#include <mgr/bitblit.h>
#include <mgr/share.h>
#include <strings.h>

#include "putshort.h"
/*}}}  */

/*{{{  variables*/
static unsigned short args[10];
static char line[80];
static char *digits = "0123456789ABCDEF";
static int t_offset = 0;
/*}}}  */

/*{{{  fputnshort write an array of short ints*/
static void fputnshort(short int *p, int count, FILE *fp)
{
  while (count--)
  {
    fputshort(*p,fp);
    p++;
  }
}
/*}}}  */

/*{{{  main*/
int main(int argc, char *argv[])
{
  int c, c1, c2;
  char *p;
  unsigned int time;

  setbuf(stderr,NULL);
  while(gets(line)) 
  {
    switch (*line) 
    {
      /*{{{  B -- bit-blit*/
      case 'B':
      sscanf(line+2,"%hu %hu %hu %hu %hu %d %hu %hu %hu",&args[1],&args[3],&args[4],&args[5],&args[6],&c,&args[2],&args[7],&args[8]);
      args[0]=OPCODE(c)|T_BLIT;
      fputnshort(args,9,stdout);
      break;
      /*}}}  */
      /*{{{  W*/
      case 'W':
      sscanf(line+2,"%hu %hu %hu %hu %hu %d",&args[1],&args[2],&args[3],&args[4],&args[5],&c);
      args[0]=OPCODE(c)|T_WRITE;
      fputnshort(args,6,stdout);
      break;
      /*}}}  */
      /*{{{  S*/
      case 'S':
      sscanf(line+2,"%hu %hu %hu %hu",&args[1],&args[2],&args[3],&args[4]);
      args[0]=T_SCREEN;
      fputnshort(args,5,stdout);
      break;
      /*}}}  */
      /*{{{  . -- get data*/
      case '.':
      p = line+2;
      while (*p) 
      {
        c1 = index(digits,*p++) - digits;
        c2 = index(digits,*p++) - digits;
        putchar(((c1&0xf)<<4) + (c2&0xf));
      }
      break;
      /*}}}  */
      /*{{{  O -- offset time*/
      case 'O':
      sscanf(line+2,"%d.%d",&c1,&c2);
      t_offset = c1*100 + c2;
      break;
      /*}}}  */
      /*{{{  N -- no-op ignore (for now)*/
      case 'N':
      break;
      /*}}}  */
      /*{{{  L*/
      case 'L':
      sscanf(line+2,"%hu %hu %hu %hu %hu %d",&args[1],&args[2],&args[3],&args[4],&args[5],&c);
      args[0]=OPCODE(c)|T_LINE;
      fwrite(args,2,6,stdout);
      break;
      /*}}}  */
      /*{{{  D*/
      case 'D':
      sscanf(line+2,"%hu %hu %hu %hu",&args[1],&args[2],&args[3],&args[4]);
      args[0]=T_DATA;
      fputnshort(args,5,stdout);
      break;
      /*}}}  */
      /*{{{  T*/
      case 'T':
      sscanf(line+2,"%d.%d",&c1,&c2);
      time = t_offset + (c1*100 + c2);
      args[1]=(time>>16)&0xffff;
      args[2]=time&0xffff;
      args[0]=T_TIME;
      fputnshort(args,3,stdout);
      break;
      /*}}}  */
      /*{{{  P*/
      case 'P':
      sscanf(line+2,"%hu %hu %hu %d",&args[1],&args[2],&args[3],&c);
      args[0]=OPCODE(c)|T_POINT;
      fputnshort(args,4,stdout);
      break;
      /*}}}  */
      /*{{{  M*/
      case 'M':
      sscanf(line+2,"%hu",&args[1]);
      args[0]=T_MARK;
      fputnshort(args,2,stdout);
      break;
      /*}}}  */
      /*{{{  K*/
      case 'K':
      sscanf(line+2,"%hu",&args[1]);
      args[0]=T_KILL;
      fputnshort(args,2,stdout);
      break;
      /*}}}  */
      /*{{{  C -- bytescroll*/
      case 'C':
      sscanf(line+2,"%hu %hu %hu %hu %hu %hu",&args[1],&args[2],&args[3],&args[4],&args[5],&args[6]);
      args[0]=T_BYTESCROLL;
      fputnshort(args,7,stdout);
      break;
      /*}}}  */
      /*{{{  default*/
      default:
      fprintf(stderr,"OOps, got 0x%x\n",*line);
      break;
      /*}}}  */
    }
  }
  return 0;
}
/*}}}  */
