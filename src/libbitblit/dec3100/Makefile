#                        Copyright (c) 1988,1989 Bellcore
#                            All Rights Reserved
#       Permission is granted to copy or use this program, EXCEPT that it
#       may not be sold for profit, the copyright notice must be reproduced
#       on copies, and credit should be given to Bellcore where it is due.
#       BELLCORE MAKES NO WARRANTY AND ACCEPTS NO LIABILITY FOR THIS PROGRAM.

#	$Header: /files/src/linuxmgr/src/libbitblit/dec3100/RCS/Makefile,v 4.3 1994/01/28 21:01:44 broman Rel $
#	$Source: /files/src/linuxmgr/src/libbitblit/dec3100/RCS/Makefile,v $

#	makefile for DEC3100 blit library

START=.
OPTIONS= -DINVERT -DUNROLL
CFLAGS= -I.  $(OPTIONS)
CPP=/lib/cpp -P -C $(OPTIONS)
CFILES= blit.C bitmap.c line.c pixel.c
HFILES= bitmap.h

#	The template expansion level is specified here for building blit.c
#	0			leave all the templates intact
#	1			expand the switches
#	12			expand the bitblt templates
#	123		expand the inner loops
#	1234		expand the bit shifts
#	12345		expand the bitwise operators
#	%			Normal CPP defines/includes
#	#			These always get expanded on the first pass

PASS1=0

# This is the C pretty printer (indent doesn't always generate valid C)
INDENT=cat

blitlib.a:	blit.o bitmap.o line.o pixel.o
				ar rv blitlib.a blit.o bitmap.o line.o pixel.o
				ranlib blitlib.a

blit.c:		blit.C
				< blit.C sed "s/^[$(PASS1)]/#/" | \
					$(CPP) | \
					sed 's/^[1-9%]/#/' | \
					$(INDENT) | \
					cat -s \
				$(ZILCH) blit.c

bitmap.o blit.o line.o pixel.o:	bitmap.h Makefile

test:			test.o bitmap.o blit.o line.o
					$(CC) $(CFLAGS) -o test test.o blit.o bitmap.o line.o

test2:			test2.o bitmap.o blit.o
					$(CC) $(CFLAGS) -o test2 test2.o blit.o bitmap.o

list:
	@for i in ${CFILES} ${HFILES} Makefile README; do \
		echo "${START}/$$i"; \
	done	

clean:
	-rm -f *.o blit.c a.out core

clobber:
	-rm blitlib.a
