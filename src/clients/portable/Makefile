include ../../../Configfile

TARGETS=	$(GROPBM) hpmgr ico ify mgrload mgrplot mgrsketch mphoon pgs sh simple $(TEXMGR) text2font
ALLTARGETS=	gropbm    hpmgr ico ify mgrload mgrplot mgrsketch mphoon pgs sh simple texmgr    text2font

all:
		for i in $(TARGETS); do (cd $$i; make all); done
		(cd xbd; make -f Makefile.mgr all)

install:
		for i in $(TARGETS); do (cd $$i; make install); done
		(cd xbd; make -f Makefile.mgr install)

depend:
		for i in $(TARGETS); do (cd $$i; make depend); done
		(cd xbd; make -f Makefile.mgr depend)

clean:
		for i in $(ALLTARGETS); do (cd $$i; make clean); done
		(cd xbd; make -f Makefile.mgr clean)

clobber:
		for i in $(ALLTARGETS); do (cd $$i; make clobber); done
		(cd xbd; make -f Makefile.mgr clobber)
