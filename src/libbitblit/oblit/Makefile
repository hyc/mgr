#                        Copyright (c) 1988 Bellcore
#                            All Rights Reserved
#       Permission is granted to copy or use this program, EXCEPT that it
#       may not be sold for profit, the copyright notice must be reproduced
#       on copies, and credit should be given to Bellcore where it is due.
#       BELLCORE MAKES NO WARRANTY AND ACCEPTS NO LIABILITY FOR THIS PROGRAM.

#	$Header: /files/src/linuxmgr/src/libbitblit/oblit/RCS/Makefile,v 4.3 1994/01/28 20:59:34 broman Stab $
#	$Source: /files/src/linuxmgr/src/libbitblit/oblit/RCS/Makefile,v $

#	makefile for old (16 bit) bitblit code

CFLAGS=-O  -I..

CFILES= blit.c line.c pixel.c bitmap.c bit_on.c
OBJS= blit.o line.o pixel.o bit_on.o
HFILES= bitmap.h
OTHER=Makefile README
LIB=blitlib.a
START=.

all:		$(LIB)

# bitmap library

$(LIB): $(LIB)(blit.o) \
           $(LIB)(pixel.o) \
           $(LIB)(line.o) \
           $(LIB)(bit_on.o) \
           $(LIB)(bitmap.o)
			ranlib $(LIB)

$(LIB)(blit.o): blit.o
			ar rv $(LIB) blit.o

$(LIB)(pixel.o): pixel.o
			ar rv $(LIB) pixel.o

$(LIB)(bit_on.o): bit_on.o
			ar rv $(LIB) bit_on.o

$(LIB)(line.o): line.o
			ar rv $(LIB) line.o

$(LIB)(bitmap.o): bitmap.o
			ar rv $(LIB) bitmap.o

##########################

$(OBJS):	bitmap.h

list:
	@for i in $(CFILES) $(HFILES) $(OTHER); do \
		echo "$(START)/$$i"; \
	done	

clean:
	$(RMF) *.o $(LIB)

clobber:
