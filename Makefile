include Configfile

TARGETS=	src doc

all:
		for i in $(TARGETS) ; do (cd $$i; make all); done
                
install:
		for i in $(TARGETS) ; do (cd $$i; make install); done

depend:
		for i in src ; do (cd $$i; make depend); done

first:
		install -d $(INCLUDEDIR)
		install -d $(BINDIR)
		install -d $(LIBDIR)
		install -d $(MANDIR)/man1
		install -d $(MANDIR)/man3
		install -d $(MANDIR)/man5
		install -d $(MANDIR)/man6

clean:
		for i in $(TARGETS) ; do (cd $$i; make clean); done

clobber:
		for i in $(TARGETS) ; do (cd $$i; make clobber); done
#		rm -f Configfile

MGR-HOWTO.txt:	MGR-HOWTO.sgml
	format -Tnroff MGR-HOWTO.sgml | qroff > MGR-HOWTO.txt

#{{{}}}
