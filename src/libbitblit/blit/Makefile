#                        Copyright (c) 1988 Bellcore
#                            All Rights Reserved
#       Permission is granted to copy or use this program, EXCEPT that it
#       may not be sold for profit, the copyright notice must be reproduced
#       on copies, and credit should be given to Bellcore where it is due.
#       BELLCORE MAKES NO WARRANTY AND ACCEPTS NO LIABILITY FOR THIS PROGRAM.

#	$Header: Makefile,v 4.6 88/07/19 15:12:53 sau Exp $
#	$Source: /tmp/mgrsrc/src/blit/RCS/Makefile,v $

#		sun version of "portable assembler" bitblit stuff

.SUFFIXES:	.C

SYMFLAGS=


# Set FLAG == -g for debugger use, -O for optimization (which breaks dbx)
FLAG= -g

#must have -g for sym to work
CFLAGS= $(FLAG) -DALIGN32
START= .
HFILES= asm.h hash.h m4.h bitmap.h
CFILES= bitmap.c bit_on.c hash.c sym.c
CCFILES= blit.C line.C pixel.C 
COFILES= blit.c line.c pixel.c
OFILES= bit_on.o bitmap.o blit.o line.o pixel.o 
LIB= blitlib.a
OTHER= Makefile README

all:	$(LIB)

fast:	clean clobber $(OFILES)
	ar rv $(LIB) $(OFILES)
	ranlib $(LIB)

# bitmap library

$(LIB): $(LIB)(bitmap.o) \
           $(LIB)(bit_on.o) \
           $(LIB)(blit.o) \
           $(LIB)(line.o) \
           $(LIB)(pixel.o)
			ranlib $(LIB)

$(LIB)(bitmap.o): bitmap.o
			ar rv $(LIB) bitmap.o

$(LIB)(bit_on.o): bit_on.o
			ar rv $(LIB) bit_on.o

$(LIB)(blit.o): blit.o
			ar rv $(LIB) blit.o

$(LIB)(line.o): line.o
			ar rv $(LIB) line.o

$(LIB)(pixel.o): pixel.o
			ar rv $(LIB) pixel.o

##########################

sym:		sym.o hash.o
			cc -o sym sym.o hash.o

blit.o:	blit.so
			cp blit.so blit.s
			cc -c -o blit.o blit.s

blit.so:	blit.sg
			if [ x$(FLAG) = x-g ]; then \
				cp blit.sg blit.so; \
			else \
				/lib/c2 -20 <blit.sg >blit.so; \
			fi

blit.sg:	blit.S sym
			./sym $(SYMFLAGS) <blit.S  |  tr '!' '_' >blit.sg

blit.S:	blit.c bitmap.h
			cc -g -S blit.c
			mv blit.s blit.S

blit.c:	blit.C asm.h m4.h
			m4 m4.h blit.C >blit.c

pixel.so:	pixel.sg
#  doesn't work!	/lib/c2 -20 <pixel.sg >pixel.so
			cp  pixel.sg pixel.so

pixel.o:	pixel.so
			cp pixel.so pixel.s
			cc -c -o pixel.o pixel.s

pixel.sg:	pixel.S sym
			./sym $(SYMFLAGS) <pixel.S  |  tr '!' '_' >pixel.sg

pixel.S:	pixel.c bitmap.h
			cc -g -S pixel.c
			mv pixel.s pixel.S

pixel.c:	pixel.C asm.h m4.h
			m4 m4.h pixel.C >pixel.c


line.so:	line.sg
			if [ x$(FLAG) = x-g ]; then \
				cp line.sg line.so; \
			else \
				/lib/c2 -20 <line.sg >line.so; \
			fi

line.o:	line.so
			cp line.so line.s
			cc -c -o line.o line.s

line.sg:	line.S sym
			./sym $(SYMFLAGS) <line.S  |  tr '!' '_' >line.sg

line.S:	line.c bitmap.h
			cc -g -S line.c
			mv line.s line.S

line.c:	line.C asm.h m4.h
			m4 m4.h line.C >line.c


$(OFILES):	bitmap.h

list:
	@for i in ${HFILES} ${CCFILES} ${CFILES} ${OTHER}; do \
		echo "${START}/$$i"; \
	done	

clean:
	$(RMF) *.[sSo] *.s[go] ${COFILES} ${LIB}

clobber:
	$(RMF) sym
