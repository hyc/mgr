include ../../../../Configfile

_CPPFLAGS=	$(CPPFLAGS) -DHFONTDIR=\"$(HFONTDIR)\" -DMGR
CFLAGS=		$(CCFLAGS) $(_CPPFLAGS)

all:		gropbm widths

install:	all
		install -c -s gropbm $(BINDIR)
		./mkdevpbm $(HFONTDIR) $(GROFFFONTDIR)
		install -c -m 644 gropbm.1 $(MANDIR)/man1

depend:		special.c
		$(MKDEP) $(MKDEPFLAGS) $(_CPPFLAGS) *.c > $(MKDEPOUT)

clean:
		$(RMF) *.o special.c

clobber:	clean
		$(ZILCH) $(DEPFILE)
		$(RMF) core gropbm widths fontview table

gropbm:		gropbm.o bit.o hfont.o
		$(CC) -o $@ $(LDFLAGS) gropbm.o bit.o hfont.o

gropbm.o:	special.c

special.c:	table
		./table < $(HFONTDIR)/special.s > special.c
		./table < $(HFONTDIR)/greek.s >> special.c

widths:		widths.c
		$(CC) -o $@ widths.c

fontview:	fontview.o
		$(CC) -o $@ $(LDFLAGS) fontview.o $(LIBDIR)/libmgr.a

include $(DEPFILE)
