#                       Copyright (c) 1988,1989 Bellcore
#                            All Rights Reserved
#       Permission is granted to copy or use this program, EXCEPT that it
#       may not be sold for profit, the copyright notice must be reproduced
#       on copies, and credit should be given to Bellcore where it is due.
#       BELLCORE MAKES NO WARRANTY AND ACCEPTS NO LIABILITY FOR THIS PROGRAM.
#
# simple multiple window multiplexor

START=.
FLAGS=$(CHECK) -L/usr/mgr/lib
MGR=/usr/mgr
CMD_DIR=/usr/mgr/mtx/scripts
CFLAGS=-I$(MGR)/include -DHOME=\"$(CMD_DIR)\" $(FLAGS)
CC=gcc  $(CHECK)

OBJS = getshell.o mtx.o mtx_connect.o mtx_net.o mtx_subs.o mtx_hosts.o \
			do_sh.o do_func.o message.o debug.o command.o mgr.o color.o

LIST= Makefile README make_root mtx_net.c mtx.1 mtx.c mtx.h mtx_subs.c \
				mtx_hosts.c getshell.c mtx_connect.c do_sh.c do_func.c message.c \
				debug.c command.c mgr.c color.c color.h *.mtx

mtx:	$(OBJS)
		$(CC) $(FLAGS) -o mtx $(OBJS) -lmgr
		cp mtx _mtx
#		-./make_root mtx

net:	net.o mtx_net.o
		$(CC) $(FLAGS) -o net net.o mtx_net.o

mtx_forward:	mtx_forward.o mtx_forward.o
		$(CC) $(FLAGS) -o mtx_forward mtx_forward.o

tcp:	tcp.o tcp.o
		$(CC) $(FLAGS) -o tcp tcp.o mtx_net.o

list:
	@for i in $(LIST); do \
		echo "$(START)/$$i"; \
	done

slist:
		-ls mtx_net.c mtx.c mtx.h mtx_subs.c \
				mtx_hosts.c getshell.c mtx_connect.c do_sh.c do_func.c message.c \
				debug.c command.c

cpio:
		$(MAKE) -s list | cpio -ocv > mtx.cpio

lint:
		lint $(LFLAGS) -o mtx  -I/usr/mgr/include \
				mtx.c mtx_connect.c mtx_net.c mtx_subs.c $(MGR)/lib//term.c
print:
		print -p -m1 -t3 -outline -2 `$(MAKE) -s list`

$(OBJS):		mtx.h
