include ../../../Configfile

CFLAGS=		$(CPPFLAGS) $(CCFLAGS)

PRGS=		rotate set_console

all:		$(PRGS)

rotate:		rotate.c
		$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $@.c $(LIBDIR)/libbitblit.a

set_console:	set_console.c
		$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $@.c $(LIBDIR)/libmgr.a

install:        all
		install -c -s $(PRGS) $(BINDIR)
		install -c -m 644 *.1 $(MANDIR)/man1

depend:
		$(MKDEP) $(MKDEPFLAGS) $(CPPFLAGS) *.c > $(MKDEPOUT)

clean:

clobber:	clean
		$(RMF) core $(PRGS) set_console
		$(ZILCH) $(DEPFILE)

include $(DEPFILE)
