include ../../../Configfile

_CPPFLAGS=	$(CPPFLAGS)
CFLAGS=		$(CCFLAGS) $(_CPPFLAGS) -DOLDMGR

PRGS=		ether stat overlay set_colormap setcolormap2 invert_color

all:		$(PRGS)

ether:		ether2.o
		$(CC) $(LDFLAGS) -o $@ $@2.o $(LIBDIR)/libmgr.a

overlay:	overlay.o
		$(CC) $(LDFLAGS) -o $@ $@.o $(LIBDIR)/libmgr.a

invert_color: invert_color.o
		$(CC) $(LDFLAGS) -o $@ $@.o -lpixrect

set_colormap:	set_colormap.o
		$(CC) $(LDFLAGS) -o $@ $@.o -lpixrect

setcolormap2:	setcolormap2.o
		$(CC) $(LDFLAGS) -o $@ $@.o $(LIBDIR)/libmgr.a $(LIBDIR)/libbitblit.a -lpixrect

stat:		stat2.o
		$(CC) $(LDFLAGS) -o $@ $@2.o $(LIBDIR)/libmgr.a

install:        all
		install -s -c $(PRGS) $(BINDIR)
		for i in *.1; do install -c -m 644 $$i $(MANDIR)/man1; done

depend:
		$(MKDEP) $(MKDEPFLAGS) $(_CPPFLAGS) *.c > $(MKDEPOUT)

clean:
		$(RMF) *.o

clobber:	clean
		$(RMF) core $(PRGS)
		$(ZILCH) $(DEPFILE)

include $(DEPFILE)
