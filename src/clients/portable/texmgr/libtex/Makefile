include ../../../../../Configfile

_CPPFLAGS=	$(CPPFLAGS) -DFONTDESC=\"$(FONTDESC)\" $(FONT_DPI_DEF)
CFLAGS=		$(CCFLAGS) $(_CPPFLAGS)

OBJS=		conv.o dviclass.o error.o findpost.o fio.o font.o font_subr.o \
		gfclass.o gripes.o magfactor.o rotate.o scaletfm.o scanpost.o \
		search.o seek.o split.o strsave.o tfm.o gffont.o pkfont.o \
		pxlfont.o tfmfont.o dvi_draw.o dvistuff.o

all:		libtex.a

libtex.a:	$(OBJS)
		$(RMF) $@
		ar cq $@ $(OBJS)
		ranlib libtex.a
                
clean:
		$(RMF) $(OBJS)
                
clobber:	clean
		$(RMF) libtex.a
