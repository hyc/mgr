#include <stdio.h>

short int fgetshort(FILE *fp)
{
  register int a = fgetc(fp);
  register int b = fgetc(fp);

  return (a==EOF || b==EOF ? EOF : (a<<8)|b);
}

int fgetnshort(FILE *fp, short int *p, int n)
{
  while (--n>0) *p++=fgetshort(fp);
  return (*p++=fgetshort(fp));
}
/*{{{}}}*/
