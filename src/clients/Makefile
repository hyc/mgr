include ../../Configfile

PRGS=		portable icon misc bitblit $(MACHDEPCLIENTS)
TARGETS=	$(PRGS) $(EXAMPLES) $(MOVIECLIENTS) $(TESTCLIENTS)
ALLTARGETS=	$(PRGS) examples    movie           tests

all:
		for i in $(TARGETS); do (cd $$i; make all); done

install:
		for i in $(TARGETS); do (cd $$i; make install); done

depend:
		for i in $(TARGETS); do (cd $$i; make depend); done

clean:
		for i in $(ALLTARGETS); do (cd $$i; make clean); done

clobber:
		for i in $(ALLTARGETS); do (cd $$i; make clobber); done
