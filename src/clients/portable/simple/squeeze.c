/*{{{}}}*/
/*{{{  Notes*/
/*

Read an modern bitmap file and do byte run-length encoding.

*/
/*}}}  */
/*{{{  #includes*/
#include <mgr/mgr.h>
#include <stdio.h>
/*}}}  */

int main(int argc, char *argv[])
{
  struct b_header unsqueezed, squeezed;
  int h,w,d;

  char bb[128], *bp, *b;
  int oc= EOF, c, nrep;
  bp=bb;

  /*{{{  read header*/
  if (fread((void*)&unsqueezed,sizeof(struct b_header),1,stdin)!=1)
  {
    fprintf(stderr,"%s: Can't read bitmap header.\n",argv[0]);
    exit(1);
  }
  /*}}}  */
  /*{{{  check if already squeezed*/
  if (BS_ISHDR(&unsqueezed))
  {
    fprintf(stderr,"%s: bitmap is already squeezed.\n",argv[0]);
    exit(1);
  }
  /*}}}  */
  /*{{{  check if old bitmap format*/
  if (!B_ISHDR8(&unsqueezed))
  {
    fprintf(stderr,"%s: Old bitmap format.\n",argv[0]);
    exit(1);
  }
  /*}}}  */
  /*{{{  write squeezed header*/
  B_GETHDR8(&unsqueezed,w,h,d);
  BS_PUTHDR(&squeezed,w,h,d);
  fwrite((void*)&squeezed,sizeof(struct b_header),1,stdout);
  /*}}}  */

  nrep=0;
  while (1)
  {
    c =getchar();
    if (c==oc)
    {
      if (nrep>126)
      {
        if (bp>bb)
        {
          putchar(127+bp-bb);
          for(b=bb; b<bp; b++)
          putchar(*b);
          bp=bb;
        }
        putchar(nrep);
        putchar(oc);
        nrep=0;
      }
      nrep++;
    }
    else
    {
      if (nrep>2)
      {
        if (bp>bb)
        {
          putchar(127+bp-bb);
          for(b=bb; b<bp; b++)
          putchar(*b);
          bp=bb;
        }
        putchar(nrep);
        putchar(oc);
        nrep=0;
      }
      while (nrep--)
      *bp++ = oc;
      if (bp-bb>125)
      {
        putchar(127+bp-bb);
        for(b=bb; b<bp; b++)
        putchar(*b);
        bp=bb;
      }
      oc=c; nrep=1;
    }
    if (c==EOF) break;
  }
  if (bp>bb)
  {
    putchar(127+bp-bb);
    for(b=bb; b<bp; b++)
    putchar(*b);
  }
}
