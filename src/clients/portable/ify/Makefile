include ../../../../Configfile

CFLAGS=		$(CCFLAGS) $(CPPFLAGS)

all:		ify setname

install:	all
		install -c -s ify setname $(BINDIR)
		install -c -m 644 setname.1 ify.1 $(MANDIR)/man1

ify:		ify.o
		$(CC) -o $@ ify.o $(LIBDIR)/libmgr.a

setname:	setname.o
		$(CC) -o setname setname.o $(LIBDIR)/libmgr.a

depend:
		$(MKDEP) $(MKDEPFLAGS) $(CPPFLAGS) *.c > $(MKDEPOUT)

clean:
		$(RMF) ify.o setname.o

clobber:	clean
		$(RMF) core setname ify
		$(ZILCH) $(DEPFILE)

include $(DEPFILE)
