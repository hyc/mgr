include ../../../../Configfile

_CPPFLAGS=	$(CPPFLAGS) -DSCREEN_DEV=\"$(SCREEN_DEV)\"
CFLAGS=		$(CCFLAGS) $(_CPPFLAGS)

all:            colormap fade screendump mgrlock mgrmag

colormap:	colormap.o
		$(CC) $(LDFLAGS) -o $@ $@.o $(LIBDIR)/libbitblit.a $(LIBDIR)/libmgr.a $(EXTRALIBS)

fade:		fade.o
		$(CC) $(LDFLAGS) -o $@ $@.o $(LIBDIR)/libbitblit.a $(EXTRALIBS)

screendump:	screendump.o
		$(CC) $(LDFLAGS) -o $@ $@.o $(LIBDIR)/libbitblit.a $(EXTRALIBS)

# snap not installed
snap:		snap.o
		$(CC) $(LDFLAGS) -o $@ $@.o $(LIBDIR)/libbitblit.a $(LIBDIR)/libmgr.a

mgrlock:	mgrlock.o
		$(CC) $(LDFLAGS) -o $@ $@.o $(CRYPTLIB) $(LIBDIR)/libbitblit.a $(LIBDIR)/libmgr.a $(EXTRALIBS)

mgrmag:		mgrmag.o
		$(CC) $(LDFLAGS) -o $@ $@.o $(LIBDIR)/libbitblit.a $(LIBDIR)/libmgr.a $(EXTRALIBS)

install:	all
		install -c -s -m $(BITBLIT_PERM) colormap fade screendump mgrlock mgrmag $(BINDIR)
		install -c -m 644 fade.1 screendump.1 mgrlock.1 mgrmag.1 $(MANDIR)/man1

depend:
		$(MKDEP) $(MKDEPFLAGS) $(_CPPFLAGS) *.c > $(MKDEPOUT)

clean:
		$(RMF) colormap.o fade.o screendump.o mgrlock.o mgrmag.o snap.o

clobber:	clean
		$(RMF) core colormap fade screendump mgrlock mgrmag snap
		$(ZILCH) $(DEPFILE)

include $(DEPFILE)
