include ../../../../Configfile

VIAFILE= -DVIAFILE
#OLDMGR = -DOLDMGR

##CFLAGS =	$(DEBUG) $(ENDIAN) $(VIAFILE) $(OLDMGR) -I$(HOME)/include
CFLAGS	=	$(VIAFILE) $(OLDMGR) $(CPPFLAGS) $(CCFLAGS)

ODIR	=	.
LDFLAGS=	$(DEBUG) -L$(LIBDIR) -lmgr -lm

OBJS   =	mphoon.o phase.o atan2.o i386.o $(SYSTEM).o
SMALL  =	smallmoon.o $(OBJS)
NORMAL =	moon.o      $(OBJS)
BIG    =	bigmoon.o   $(OBJS)
BINS	=	$(ODIR)/mphoon_small $(ODIR)/mphoon $(ODIR)/mphoon_big

all:		$(BINS)
		@echo "Mphoon programs made"

install:	all
		install -s $(BINS) $(BINDIR)
		install -m 644 mphoon.man $(MANDIR)/man1/mphoon.1

depend:
		$(MKDEP) $(MKDEPFLAGS) $(CPPFLAGS) *.c > $(MKDEPOUT)

clean:
		$(RMF) *.o core

clobber:	clean
		$(RMF) $(BINS)
		$(ZILCH) $(DEPFILE)

$(ODIR)/mphoon_small: $(SMALL)
		$(CC) $(SMALL)  $(LDFLAGS) -o $@

$(ODIR)/mphoon:       $(NORMAL)
		$(CC) $(NORMAL) $(LDFLAGS) -o $@

$(ODIR)/mphoon_big:   $(BIG)
		$(CC) $(BIG)    $(LDFLAGS) -o $@

include $(DEPFILE)
