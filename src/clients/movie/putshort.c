#include <stdio.h>

short int fputshort(short int n, FILE *fp)
{
  short int x;

  fputc(n>>8,fp);
  return((x=fputc(n&0xff,fp))<0 ? x : n);
}
/*{{{}}}*/
