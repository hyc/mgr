#                        Copyright (c) 1988 Bellcore
#                            All Rights Reserved
#       Permission is granted to copy or use this program, EXCEPT that it
#       may not be sold for profit, the copyright notice must be reproduced
#       on copies, and credit should be given to Bellcore where it is due.
#       BELLCORE MAKES NO WARRANTY AND ACCEPTS NO LIABILITY FOR THIS PROGRAM.

#	$Header: Makefile,v 4.3 88/08/10 17:31:14 bianchi Exp $
#	$Source: /tmp/mgrsrc/src/pixrect/RCS/Makefile,v $

#	makefile for old (16 bit) bitblit code

CFLAGS=-O

CFILES= sundep.c
OFILES= sundep.o
HFILES= bitmap.h
OTHER=Makefile
LIB=blitlib.a
START=.

all:		$(LIB)

# bitmap library

$(LIB): \
           $(LIB)(sundep.o)
			ranlib $(LIB)

$(LIB)(sundep.o): sundep.o
			ar rv $(LIB) sundep.o

sundep.o: \
	sundep.c bitmap.h 

##########################


list:
	@for i in $(HFILES) $(CFILES) $(OTHER); do \
		echo "$(START)/$$i"; \
	done	

clean:
	rm -f $(OFILES) $(LIB)

clobber:
