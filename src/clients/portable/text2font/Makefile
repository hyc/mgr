include ../../../../Configfile

CFLAGS=		$(CCFLAGS) $(CPPFLAGS)

PRGS=		font2text text2font

all:		$(PRGS)

text2font:	text2font.c
		$(CC) $(CFLAGS) -o $@ $@.c $(LIBDIR)/libmgr.a

font2text:	font2text.c
		$(CC) $(CFLAGS) -o $@ $@.c $(LIBDIR)/libmgr.a

install:        all
		install -c -s $(PRGS) $(BINDIR)
		for i in $(PRGS); do install -c -m 644 $$i.1 $(MANDIR)/man1; done

depend:
		$(MKDEP) $(MKDEPFLAGS) $(CPPFLAGS) *.c > $(MKDEPOUT)

clean:

clobber:
		$(RMF) core $(PRGS)
		$(ZILCH) $(DEPFILE)

include $(DEPFILE)
