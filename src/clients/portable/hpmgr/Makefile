include ../../../../Configfile

CFLAGS=		$(CCFLAGS) $(CPPFLAGS)

OBJS=		hpmgr.o getpty.o

all:		hpmgr

hpmgr:		$(OBJS)
		$(CC) $(LDFLAGS) -o $@ $(OBJS) $(LIBDIR)/libmgr.a

install:        all
		install -s hpmgr $(BINDIR)
		install -c -m 644 hpmgr.1 $(MANDIR)/man1

depend:
		$(MKDEP) $(MKDEPFLAGS) $(CPPFLAGS) *.c > $(MKDEPOUT)

clean:
		$(RMF) $(OBJS)

clobber:	clean
		$(RMF) core hpmgr
		$(ZILCH) $(DEPFILE)

include $(DEPFILE)
