include ../Configfile

# these are usually outcommented
#ROFFDEVICE=	-Tpbm -P-v -P-x -P360 -P-y -P360 -P-c -P'(/usr/pbmplus/bin/pbmtoprt -p eps360x360 | lp -p A)'
#ROFFDEVICE=	-Tpbm -P-x -P100 -P-y -P90 -P-m -P-c -P'(cat >/tmp/gropbm$$; mgrview -r /tmp/gropbm$$; rm /tmp/gropbm$$)'
#ROFFDEVICE=	-Tpbm -Z
#PAGELEN=	p10
#PAGES=		-o41-

DOCUMENT=	doc.1 doc.2 doc.3 doc.4 doc.5 doc.6
CRFLAGS=	-k '' '' -c '\fI' '\fP'

all:		manual.out

install:	all

manual.out:	abstract.out body.out
		cat abstract.out body.out > manual.out

body.out:	macros $(DOCUMENT) tocindex
		$(TBL) $(PAGELEN) macros $(DOCUMENT) tocindex doc.9 | croff/croff $(CRFLAGS) | $(ROFF) $(ROFFDEVICE) $(PAGES) $(MS) -rI0 > body.out

abstract.out:	macros abstract.ms
		$(ROFF) $(ROFFDEVICE) $(MS) -rI0 macros abstract.ms > abstract.out

tocindex:	macros $(DOCUMENT) croff/croff
		$(TBL) $(PAGELEN) macros $(DOCUMENT) | croff/croff $(CRFLAGS) | $(ROFF) $(MS) -rI1 $(ROFFDEVICE) $(ROFFONLY) >/dev/null 2> tmpindex
		grep '^\.Fc' tmpindex | sort +2 -n > tocindex
		grep '^[a-z]' tmpindex | sort +0 -1 -d +1 -2 -n +2 -3 -r | $(AWK) -f index.awk >> tocindex
		$(RMF) tmpindex

croff/croff:
		cd croff; make install

clean:
		$(RMF) tocindex tmpindex abstract.out body.out
		cd croff; make clean

clobber:	clean
		$(RMF) manual.out
		cd croff; make clobber
