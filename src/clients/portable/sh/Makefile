include ../../../../Configfile

PRGS=		bury c_menu cursor color font loadfont shape square menu resetwin

all:

install:	all
		install -d $(BINDIR)
		install -c $(PRGS) $(SHBINDIR)
		for i in $(PRGS); do install -c -m 644 $$i.1 $(MANDIR)/man1; done

depend:

clean:

clobber:	clean
