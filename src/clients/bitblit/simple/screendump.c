/*{{{}}}*/
/*{{{  #includes*/
#include <mgr/bitblit.h>
#include <unistd.h>
#include <getopt.h>
#include <stdio.h>
/*}}}  */

/*{{{  main*/
int main(int argc, char *argv[])
{
  /*{{{  variables*/
  char *screen_name=SCREEN_DEV, *snap_name="";
  FILE *snap_fp=stdout;
  BITMAP *screen, *snap;
  int c;
  /*}}}  */

  /*{{{  parse arguments*/
  while ((c=getopt(argc,argv,"s:o:"))!=EOF)
  {
    switch (c)
    {
      /*{{{  o*/
      case 'o': snap_name=optarg; break;
      /*}}}  */
      /*{{{  s*/
      case 's': screen_name=optarg; break;
      /*}}}  */
      /*{{{  ?*/
      case '?':
      {
        fprintf(stderr,"Usage: %s [-s screen][-o file]\n",argv[0]);
        exit(1);
        break;
      }
      /*}}}  */
    }
  }
  /*}}}  */
  /*{{{  open screen*/
  if ((screen=bit_open(screen_name))==(BITMAP*)0)
  {
    fprintf(stderr,"%s: Can't open screen.\n",argv[0]);
    exit(1);
  }
  /*}}}  */
  /*{{{  reset suid status*/
  setuid(getuid()); setgid(getgid());
  /*}}}  */
  /*{{{  open snap file (if any)*/
  if (*snap_name && (snap_fp=fopen(snap_name,"w"))==(FILE*)0)
  {
    fprintf(stderr,"%s: Can't open %s.\n",argv[0],optarg);
    exit(1);
  }
  /*}}}  */
  /*{{{  alloc snap bitmap*/
  if ((snap=bit_alloc(BIT_WIDE(screen),BIT_HIGH(screen),(BITMAP*)0,BIT_DEPTH(screen)))==(BITMAP*)0)
  {
    fprintf(stderr,"%s: Can't open screen.\n",argv[0]);
    exit(1);
  }
  /*}}}  */
  /*{{{  copy screen to snap*/
  bit_blit(snap,0,0,BIT_WIDE(screen),BIT_HIGH(screen),BIT_SRC,screen,0,0);
  /*}}}  */
  /*{{{  write snap*/
  bitmapwrite(snap_fp,snap);
  if (snap_fp!=stdout) fclose(snap_fp);
  /*}}}  */
  return 0;
}
/*}}}  */
