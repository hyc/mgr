/*{{{  m_parse -- parse a line into fields*/
#ifndef iswhite
#define iswhite(x)	((x)==' ' || (x)=='\t')
#endif

extern int
m_parse(line,fields)
register char *line;
register char **fields;
   {
   int inword = 0;
   int count = 0;
   char *start;
   register char c;

   for(start = line;(c = *line) && c != '\n';line++)
      if (inword && iswhite(c)) {
         inword = 0;
         *line = '\0';
         *fields++ = start;
         count++;
         }
      else if (!inword && !iswhite(c)) {
         start = line;
         inword = 1;
         }

   if (inword) {
      *fields++ = start;
      count++;
      if (c == '\n')
         *line = '\0';
      }
   *fields = (char *) 0;
   return(count);
   }
/*}}}  */

