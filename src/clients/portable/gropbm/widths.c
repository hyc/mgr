#include <stdio.h>

int main(int argc, char *argv[])
{
  char ln[1024];
  int n=0, width;
  int iso=!strcmp(argv[2],"iso");
  int special=!strcmp(argv[2],"special");

  printf("name %s\n",argv[1]);
  if (special) printf("special\n");
  while (fgets(ln,sizeof(ln),stdin)!=(char*)0)
  {
    width=ln[9]-ln[8];
    if (n==0) printf("spacewidth %d\ncharset\n",width);
    else if (ln[0]==' ' || ln[1]==' ') printf("%c	%d	0	%d\n",n+' ',width,n+' ');
    else
    {
      printf("%c%c	%d	0	%d\n",ln[0],ln[1],width,n+' ');
      if (iso) switch (n+' ')
      {
        case 129: printf("char228	\"\n"); break;
        case 128: printf("char196	\"\n"); break;
        case 131: printf("char246	\"\n"); break;
        case 130: printf("char214	\"\n"); break;
        case 133: printf("char252	\"\n"); break;
        case 132: printf("char220	\"\n"); break;
        case 134: printf("char223	\"\n"); break;
      }
    }
    n++;
  }
  exit(0);
}
/*{{{}}}*/
