include ../../../../Configfile

CFLAGS=		$(CCFLAGS) $(CPPFLAGS)

all:		ico

install:	all
		install -c -s ico $(BINDIR)
		install -c -m 644 ico.1 $(MANDIR)/man1

depend:
		$(MKDEP) $(MKDEPFLAGS) $(CPPFLAGS) *.c > $(MKDEPOUT)

ico:		ico.o
		$(CC) -o $@ ico.o -lm $(LIBDIR)/libmgr.a

clean:
		$(RMF) ico.o

clobber:	clean
		$(RMF) core ico
		$(ZILCH) $(DEPFILE)

include $(DEPFILE)
