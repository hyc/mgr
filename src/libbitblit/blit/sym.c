/*                        Copyright (c) 1988 Bellcore
 *                            All Rights Reserved
 *       Permission is granted to copy or use this program, EXCEPT that it
 *       may not be sold for profit, the copyright notice must be reproduced
 *       on copies, and credit should be given to Bellcore where it is due.
 *       BELLCORE MAKES NO WARRANTY AND ACCEPTS NO LIABILITY FOR THIS PROGRAM.
 */
/*	$Header: sym.c,v 4.1 88/06/21 13:19:15 bianchi Exp $
	$Source: /tmp/mgrsrc/src/blit/RCS/sym.c,v $
*/
static char	RCSid_[] = "$Source: /tmp/mgrsrc/src/blit/RCS/sym.c,v $$Revision: 4.1 $";

/* do symbolic substitution for asm decls SUN version */

/*	sample stab entries
	.stabs	"x:r(0,1)",0x40,0,4,7
	.stabs	"i:r(0,1)",0x40,0,4,6
	.stabs	"d:r(0,13)",0x40,0,1,12
	.stabs	"s:r(0,13)=*(0,2)",0x40,0,1,13
   sample sun instruction
	   bfins.l %d1, (%$dbase)[%$dst:%$field]
*/

#include "stdio.h"
#include "hash.h"

#define GET_OPT(i)   \
   strlen(argv[i])>2 ? argv[i]+2 : argv[++i]

#define FMT	"	.stabs	\"%[^:]:r%*[^\"]\",0x%d,%*d,%*d,%s"
#define TSIZE		501			/* # of entries in hash table */
#define KEY	"#$ %s %s"		/* key work entry */
#define dprintf	if(debug)fprintf

char line[512];			/* input line buffer */
char name[32];				/* parameter name */
char reg[10];				/* register value */

struct table_entry *table[TSIZE];

main(argc,argv)
int argc;
char **argv;
   {
   int class;		/* storage class */
   int n;
   char *start, *end;	/* for finding sub's. */
   char *index(), *get_end();
   int value;		/* register value to substitute */
   int count = 0;	/* # of subs */
   int line_cnt=0;		/* input line # */
   char type;		/* register type 'a' or 'd' */
   int debug = 0;
   register int i;

   for(i=1;i<argc;i++) {
      if (*argv[i] == '-')
         switch (argv[i][1]) {
            case 'd':            /* turn on debugging */
               debug++;
               break;
            case 'K':            /* add keyword */
                  { 
                  char *key = (GET_OPT(i));
                  char *reg = argv[++i];
                  add_entry(table,TSIZE,key);
                  put_entry(table,TSIZE,key,reg);
                  dprintf(stderr,"adding %s => %s from command line\n",key,reg);
                  }
               break;
            default:
               fprintf(stderr,"%s: bad flag %c ignored\n",argv[0],argv[i][1]);
            }
         }

   dprintf(stderr,"Asm symbolic processor\n");
   while(gets(line) != NULL) {
      line_cnt++;
		n = sscanf(line,FMT,name,&class,reg);
		if (n==3 && class == 40) {					/* declaration */
         if (is_entry(table,TSIZE,reg))  {		/* delete old name */
            dlt_entry(table,TSIZE,get_entry(table,TSIZE,reg));
            }
         add_entry(table,TSIZE,reg);
         add_entry(table,TSIZE,name);
         put_entry(table,TSIZE,name,reg);
         put_entry(table,TSIZE,reg,name);
         puts(line);
         dprintf(stderr,"adding %s => %s from stab entry\n",name,reg);
         }
      else if (sscanf(line,KEY,name,reg) == 2) {	/* keyword substitution */
         add_entry(table,TSIZE,name);
         put_entry(table,TSIZE,name,reg);
         dprintf(stderr,"adding %s => %s from file\n",name,reg);
         }
      else if (start = index(line,'$')) {				/* parameter substitution */
         fwrite(line,1,start-line,stdout);
         do {
            end = get_end(start+1);
            strncpy(name,start+1,end-start-1);
            name[end-start-1] = '\0';
            if (is_entry(table,TSIZE,name)) { /* do substitution */
               value = atoi(get_entry(table,TSIZE,name));
               if (value > 7) {
                  type = 'a';
                  value -=8;
                  }
               else
                  type = 'd';
               printf("%c%d",type,value);
               dprintf(stderr,"line %d: substituting <%c%d> for <%s>\n",
                       line_cnt,type,value,name);
               count++;
               }
            else if (is_reg(name))	{							/* can't find sub */
               printf("%s",name);
               } 
            else {			
               printf("$%s",name);
               fprintf(stderr,"%s: Line %d no register found for %s\n",
                       *argv,line_cnt,name);
               }
            if (start = index(end,'$')) 
               fwrite(end,1,start-end,stdout);
            else
               printf("%s\n",end);
            }
         while(start);
         }
      else															/* pass through */
         puts(line);
      }
   dprintf(stderr,"%d substitutions made on %d lines\n",count,line_cnt);
   exit(0);
   }

/* find end of word */

char *
get_end(str)
char *str;
   {
   register char c;
   register int i=0;

   while (i++,(c = *str++)) {
      if (c>='a' && c<='z') continue;
      if (c>='A' && c <= 'Z') continue;
      if (i>1 && c>='0' && c <= '9') continue;
      if (c != '_') break;
      }
   return(str-1);
   }

/* see if variable is [ad][0-7] */

int
is_reg(name)
char *name;
   {
   return( (name[0]=='d' || name[0] == 'a') &&
           (name[1] >= '0' && name[1] <= '7'));
   }
