# Makefile for pgs, the MGR ghostscript interpreter

include ../../../../Configfile

LIBMGR = $(LIBDIR)/libmgr.a
CFLAGS= $(CCFLAGS) $(CPPFLAGS)

all:	pgs

pgs:	pgs.o $(LIBMGR)
	$(CC) -o pgs pgs.o $(LIBMGR)

install:	pgs
	install -c -s pgs $(BINDIR)
	install -c -m 644 pgs.1 $(MANDIR)/man1

depend:
	$(MKDEP) $(MKDEPFLAGS) $(CPPFLAGS) *.c > $(MKDEPOUT)

clean:
	$(RMF) *.o core

clobber: clean
	$(RMF) pgs
	$(ZILCH) $(DEPFILE)

include $(DEPFILE)
