include ../../../../Configfile

# The following defaults for TEXMGR flags will be used unless overriden
#   -DPAGE_HEIGHT=11				/* inches */
#   -DPAGE_WIDTH=8.5				/* inches */
#   -DFONT_DPI=300
#   -DBLACKNESS=3

_CPPFLAGS=	$(CPPFLAGS) -Ilibtex -DFONTDESC=\"$(FONTDESC)\" $(FONT_DPI_DEF)
CFLAGS=		$(CCFLAGS) $(_CPPFLAGS)

OBJS=		texmgr.o misc.o libtex/libtex.a

all:		texmgr

texmgr:		$(OBJS)
		$(CC) $(LDFLAGS) -o texmgr $(OBJS) $(LIBDIR)/libmgr.a

texmgr.o:	libtex/dvistuff.h

libtex/libtex.a:
		cd libtex; make

install:	texmgr
		install -c -s texmgr $(BINDIR)
		install -c -m 644 texmgr.1 $(MANDIR)/man1

depend:
		$(MKDEP) $(MKDEPFLAGS) $(_CPPFLAGS) *.c > $(MKDEPOUT)

clean:
		$(RMF) $(OBJS)
		(cd libtex; make clean)

clobber:
		$(RMF) $(OBJS) texmgr
		(cd libtex; make clobber)
		$(ZILCH) $(DEPFILE)

include $(DEPFILE)
