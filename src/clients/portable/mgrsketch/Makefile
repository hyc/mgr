include ../../../../Configfile

CFLAGS=		$(CCFLAGS) $(CPPFLAGS)

OBJS=		mgrsketch.o

all:		mgrsketch

mgrsketch:	$(OBJS)
		$(CC) $(LDFLAGS) -o $@ $(OBJS) $(LIBDIR)/libmgr.a -lm

install:        all
		install -s -c mgrsketch $(BINDIR)
		install -c -m 644 mgrsketch.1 $(MANDIR)/man1

depend:
		$(MKDEP) $(MKDEPFLAGS) $(CPPFLAGS) *.c > $(MKDEPOUT)

clean:
		$(RMF) $(OBJS)

clobber:	clean
		$(RMF) core mgrsketch
		$(ZILCH) $(DEPFILE)

include $(DEPFILE)
