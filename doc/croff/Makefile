include ../../Configfile

# hash table size (for keywords)
BUCKETS=	59
CFLAGS=		$(FLAGS) -DHSIZE=$(BUCKETS)

all:		croff

install:	all
		install -c -m 644 croff.1 $(MANDIR)/man1

croff:		keys.o croff.o hash.o
		$(CC) -o croff croff.o hash.o keys.o $(LEXLIB)

keys.c:		keywords mktable
		./mktable -n keywords $(BUCKETS) < keywords > keys.c

mktable:	mktable.o hash.o
		$(CC) $(CFLAGS) -o mktable mktable.o hash.o

clean:
		$(RMF) *.o core croff.c keys.c mktable

clobber:	clean
		$(RMF) croff

hash.o:		hash.h

mktable.o:	hash.h
