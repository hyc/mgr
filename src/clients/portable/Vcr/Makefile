include ../../../../Configfile

CFLAGS=		$(CCFLAGS) $(CPPFLAGS)

OBJS=           vcr.o

all:            vcr

vcr:		$(OBJS)
		$(CC) $(LDFLAGS) -o $@ $(OBJS) $(LIBDIR)/libmgr.a

install:        all
		install -s -c vcr $(BINDIR)
		install -c -m 644 vcr.1 $(MANDIR)/man1

depend:
		$(MKDEP) $(MKDEPFLAGS) $(CPPFLAGS) *.c > $(MKDEPOUT)

clean:
		$(RMF) $(OBJS)

clobber:	clean
		$(RMF) core vcr
		$(ZILCH) $(DEPFILE)

include $(DEPFILE)
