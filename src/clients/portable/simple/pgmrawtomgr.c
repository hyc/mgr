#include <mgr/mgr.h>
#include <ctype.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>

/*{{{  convert*/
void convert(int in)
{
  char ln[3],c;
  int width,height,mlev;
  size_t count;

  ln[0]='\0';
  if (read(in,ln,3)==3 && ln[0]=='P' && ln[1]=='5' && ln[2]=='\n')
  {
    width=0;
    while (read(in,&c,1)==1 && isdigit(c)) width=10*width+c-'0';
    if (c==' ')
    {
      height=0;
      while (read(in,&c,1)==1 && isdigit(c)) height=10*height+c-'0';
      if (c=='\n')
      {
	mlev = 0;
	while (read(in,&c,1)==1 && isdigit(c)) mlev=10*mlev+c-'0';
	if (c=='\n' && mlev <= 255)
	{
	  struct b_header header;
	  char buffer[8192];

	  B_PUTHDR8(&header,width,height,8);
	  write(1,&header,sizeof(header));
	  do
	  {
	    count=read(in,buffer,sizeof(buffer));
	    if (count>0 && write(1,buffer,count)!=count)
	    {
	      fprintf(stderr,"pgmrawtomgr: write error\n");
	      exit(1);
	    }
	  } while (count==sizeof(buffer));
	  return;
        }
      }
    }
  }
  fprintf(stderr,"pgmrawtomgr: No pgm raw graymap.\n");
  exit(1);
}
/*}}}  */

/*{{{  main*/
int main(int argc, char *argv[])
{
  int i,fd;

  if (argc>1) for (i=1; i<argc; i++)
  {
    if ((fd=open(argv[i],O_RDONLY))>=0)
    {
      convert(fd);
      close(fd);
    }
    else
    {
      fprintf(stderr,"pgmrawtomgr: can't open %s.\n",argv[i]);
      exit(1);
    }
  }
  else convert(0);
  return 0;
}
/*}}}  */
