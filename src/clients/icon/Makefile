include ../../../Configfile

_CPPFLAGS=	$(CPPFLAGS) -DSCREEN_DEV=\"$(SCREEN_DEV)\" -DOLDLIBMGR
CFLAGS=		$(CCFLAGS) $(_CPPFLAGS)

PRGS=		zoom

all:		$(PRGS)

zoom:		zoom.o
		$(CC) $(LDFLAGS) -o $@ $@.o $(LIBDIR)/libmgr.a $(LIBDIR)/libbitblit.a $(EXTRALIBS)

install:        all
		install -s -c $(PRGS) $(BINDIR)
		for i in $(PRGS); do install -c -m 644 $$i.1 $(MANDIR)/man1; done

depend:
		$(MKDEP) $(MKDEPFLAGS) $(_CPPFLAGS) *.c > $(MKDEPOUT)

clean:
		$(RMF) *.o

clobber:	clean
		$(RMF) core $(PRGS)
		$(ZILCH) $(DEPFILE)

include $(DEPFILE)
