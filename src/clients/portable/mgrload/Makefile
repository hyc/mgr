include ../../../../Configfile

CFLAGS=		$(CCFLAGS) $(CPPFLAGS)

all:		mgrload

mgrload:        mgrload.o getload.o
		$(CC) $(LDFLAGS) -o $@ mgrload.o getload.o $(LIBDIR)/libmgr.a

install:	all
		install -s -c mgrload $(BINDIR)
		install -c -m 644 mgrload.1 $(MANDIR)/man1

depend:
		$(MKDEP) $(MKDEPFLAGS) $(CPPFLAGS) mgrload.c getload.c > $(MKDEPOUT)

clean:
		$(RMF) mgrload.o getload.o

clobber:	clean
		$(RMF) core mgrload
		$(ZILCH) $(DEPFILE)

include $(DEPFILE)
