include ../../../../Configfile

CFLAGS=		$(CCFLAGS) $(CPPFLAGS)
OBJS=		mgrplot.o subs.o

all:		mgrplot

install:	all
		install -s mgrplot $(BINDIR)
		install -m 644 mgrplot.1 $(MANDIR)/man1

depend:
		$(MKDEP) $(MKDEPFLAGS) $(CPPFLAGS) *.c > $(MKDEPOUT)

mgrplot:	$(OBJS)
		$(CC) -o $@ $(OBJS) $(LIBDIR)/libmgr.a

clean:
		$(RMF) $(OBJS)
                
clobber:	clean
		$(RMF) core mgrplot
		$(ZILCH) $(DEPFILE)

include $(DEPFILE)
