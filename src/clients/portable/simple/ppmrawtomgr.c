#include <ctype.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <stdio.h>
#include <mgr/mgr.h>

/* rescale computes a color index from the range [0..newmax]
   corresponding to a color index supplied in the range [0..oldmax].
   0 maps to 0 and oldmax maps to newmax.
   The expanding rescaling produces nearly equispaced outputs,
   and contracting rescaling maps nearly equal intervals to a single output.
   A rescaling followed by the reverse rescaling is a projection mapping.
 */
unsigned int rescale( unsigned int col, unsigned int oldmax,
                                        unsigned int newmax) {
    unsigned int newcol;

    if( newmax >= oldmax)
        newcol = oldmax? (col*2*newmax + oldmax) / (2*oldmax): newmax;
    else
        newcol = (col*2*(newmax + 1) + newmax) / (2*(oldmax + 1));
    return newcol;
}

unsigned int pixval( unsigned char r, unsigned char g,
			unsigned char b, unsigned char maxl) {
    return (rescale( b, maxl, 3)<<6) | 
	   (rescale( r, maxl, 7)<<3) |
	   (rescale( g, maxl, 7)<<0);
}

/*{{{  convert*/
void convert(FILE *fin)
{
  char ln[3],c;
  int width,height;
  unsigned int mlev;
  size_t count;

  if (fread(ln,1,3,fin)==3 && ln[0]=='P' && ln[1]=='6' && ln[2]=='\n')
  {
    width=0;
    while (fread(&c,1,1,fin)==1 && isdigit(c)) width=10*width+c-'0';
    if (c==' ')
    {
      height=0;
      while (fread(&c,1,1,fin)==1 && isdigit(c)) height=10*height+c-'0';
      if (c=='\n')
      {
	mlev = 0;
	while (fread(&c,1,1,fin)==1 && isdigit(c)) mlev=10*mlev+c-'0';
	if (c=='\n' && mlev <= 255)
	{
	  struct b_header header;
	  char buffer[3];

	  B_PUTHDR8(&header,width,height,8);
	  fwrite(&header,sizeof(header),1,stdout);
	  do
	  {
	    count = fread(buffer,1,sizeof(buffer),fin);
	    c = pixval( buffer[0], buffer[1], buffer[2], mlev);
	    if (count==sizeof(buffer) && fwrite(&c,1,1,stdout)<1)
	    {
	      fprintf(stderr,"ppmrawtomgr: write error\n");
	      exit(1);
	    }
	  } while (count==sizeof(buffer));
	  return;
        }
      }
    }
  }
  fprintf(stderr,"ppmrawtomgr: No ppm raw pixmap.\n");
  exit(1);
}
/*}}}  */

/*{{{  main*/
int main(int argc, char *argv[])
{
  int i;
  FILE *f;

  if (argc>1) for (i=1; i<argc; i++)
  {
    if ((f=fopen(argv[i],"r"))!=NULL)
    {
      convert(f);
      fclose(f);
    }
    else
    {
      fprintf(stderr,"ppmrawtomgr: can't open %s.\n",argv[i]);
      exit(1);
    }
  }
  else convert(stdin);
  return 0;
}
/*}}}  */
