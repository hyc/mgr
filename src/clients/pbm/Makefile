# Makefile for pbmstream and pbmtoprt
#
# Default Compiler
CC=	gcc
#CC=	ccc

# Where is pbmplus installed
PBM=/usr/pbmplus

DEBUG=
#CFLAGS= ${DEBUG} -O2 -Wall -ansi -pedantic -I${PBM}/include -D_POSIX_SOURCE
CFLAGS= ${DEBUG} -O2 -Wall -fomit-frame-pointer -fstrength-reduce -I${PBM}/include -D_POSIX_SOURCE
LFLAGS= ${DEBUG} -L${PBM}/lib
LIBS=	-lpnm -lppm -lpgm -lpbm

BINDIR= $(PBM)/bin
BINS=	pbmtoprt pbmstream pnmstream

all	: ${BINS}

pbmtoprt	: pbmtoprt.o
	${CC} -o pbmtoprt ${LFLAGS} pbmtoprt.o ${LIBS}

pbmstream	: pbmstream.o
	$(CC) -o pbmstream ${LFLAGS} pbmstream.o ${LIBS}

pnmstream	: pnmstream.o
	$(CC) -o pnmstream ${LFLAGS} pnmstream.o ${LIBS}

clean	:
	$(RMF) *.o

clobber	: clean
	$(RMF) ${BINS} core

install	: ${BINS}
	for i in ${BINS}; do install -c -s $$i ${BINDIR}/$$i; done
