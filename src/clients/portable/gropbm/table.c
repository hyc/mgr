#include <stdio.h>

int main(int argc, char *argv[])
{
  char ln[1024];
  int n=0;

  while (fgets(ln,sizeof(ln),stdin)!=(char*)0)
  {
    if (n && ln[0]!=' ' && ln[1]!=' ') printf("case INT('%s%c','%c'): r[0]=%d; break;\n",ln[0]=='\\' ? "\\" : "", ln[0],ln[1],n+' ');
    n++;
  }
  return 0;
}
/*{{{}}}*/
