include ../../../Configfile

_CPPFLAGS=	$(CPPFLAGS) -DSCREEN_DEV=\"$(SCREEN_DEV)\"
CFLAGS=		$(CCFLAGS) $(CPPFLAGS)

all:		lines monorop rops

depend:
		$(MKDEP) $(MKDEPFLAGS) $(_CPPFLAGS) *.c > $(MKDEPOUT)

monorop:	monorop.o $(LIBDIR)/libbitblit.a
		$(CC) $(LDFLAGS) -o $@ monorop.o $(LIBDIR)/libbitblit.a

lines:		lines.o $(LIBDIR)/libbitblit.a
		$(CC) $(LDFLAGS) -o $@ lines.o $(LIBDIR)/libbitblit.a

rops:		rops.o $(LIBDIR)/libbitblit.a
		$(CC) $(LDFLAGS) -o $@ rops.o $(LIBDIR)/libbitblit.a

clean:
		$(RMF) lines.o lines.log monorop.o rops.o analyse.o

clobber:	clean
		$(RMF) core lines monorop rops
		$(ZILCH) $(DEPFILE)

include $(DEPFILE)
