CFLAGS=  -O
LDFLAGS= -s
SHLIB=   /lib/crt0s.o /lib/shlib.ifile

all: ft2fnt

ft2fnt: ft2fnt.o
	$(LD) $(LDFLAGS) -o $@ $(SHLIB) $?

clean:
	$(RMF) *.o *.out core

clobber: clean
	$(RMF) ft2fnt
