/*
 * m_bitc_sendh, m_bitc_sendb, m_bitc_sent.
 * These routines transfer a bitmap from client memory to the Mgr server,
 * to a scratchpad bitmap, to the current window, or to the root window pattern.
 *
 * If the server is remote, or the bitmap small, the bits are written
 * on the tty, otherwise a temporary file is used.
 * The bit data is written out in Mgr external bitmap format, without header,
 * i.e. in raster scan order, top-to-bottom and left-to-right,
 * with each line padded to a byte boundary.
 * For monochrome, the order of bits in a byte is bigendian.
 * The return value from m_bitc_sendh is 0==success, -1==failure
 *                  from m_bitc_sendb is 0==success, -1==failure
 *                  from m_bitc_sent  is 1==success,  0==failure.
 * The finalization procedure m_bitc_sent is not optional.
 *
 * Call the routines something like this:
 *	(void) m_bitc_sendh( 640, 480, 8, 3, 0, 0, 49);
 *	do {
 *	    get some more of the bitmap data into bitbuffer.
 *	} while( m_bitc_sendb( bitbuffer, howmuch) == 0 && thereismore);
 *	if( !m_bitc_sent()) complain();
 *
 * The buffering is taken care of, so m_bitc_sendb() can be called for one
 * big write or many little writes.
 * The to_tmp bitmap is clobbered only if the transfer goes
 * through a temporary file and the x,y offsets are not both zero.
 * Transfers of multiple bitmaps cannot be nested or interleaved,
 * because of the single tty available and the use of static storage.
 *
 * Author: Vincent Broman, broman@nosc.mil, Nov 1994.
 */

#include <sys/types.h>
#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <mgr/mgr.h>

#ifndef P_BITOP		/* new in server */
#define P_BITOP 0	/* a bitmap copy clobbers the m_func() bitop */
#endif

#define SMALLBMPSIZE 128
#define TTYBATCHSIZE 64

static enum {
    MBS_IDLE, MBS_BYTTY, MBS_BYFILE
} busy = MBS_IDLE;
static int bytesleft;		/* unsent byte count */

static int wide, high, deep, to_bm, offx, offy, to_tmp_bm;
static char filename[] = "/tmp/bitcsendXXXXXX";
static FILE *filep;
static char ttybuff[TTYBATCHSIZE];


/* initialize a transfer of bitmap and return 0 for success, -1 for failure. */
int m_bitc_sendh(		/* send bitmap from client to server */
		  int w,	/* width in bits */
		  int h,	/* height in bits */
		  int d,	/* depth in bits */
		  int to,	/* dest. scratchpad bitmap nbr */
		  int x,	/* x offset in destination */
		  int y,	/* y offset in destination */
		  int to_tmp	/* (free) temp bitmap, unused if x=y=0 */
		  ) {

    if( busy != MBS_IDLE) {
	errno = EBUSY;
	return -1;
    } else if( w<0 || h<0 || d<0 || to<-1 || x<0 || y<0 || to_tmp<0) {
	errno = EINVAL;
	return -1;
    }
    wide = w;
    high = h;
    deep = d;
    to_bm = to;
    offx = x;
    offy = y;
    to_tmp_bm = to_tmp;

    bytesleft = B_SIZE8( w, h, d);
    busy = (bytesleft > SMALLBMPSIZE && m_localsrv())? MBS_BYFILE: MBS_BYTTY;
    if( busy == MBS_BYFILE) {

	mktemp( filename);
	filep = fopen( filename, "w");
	if( filep) {
	    struct b_header head;

	    B_PUTHDR8( &head, w, h, d );
	    if( fwrite( (char *)&head, sizeof( head), 1, filep) != 1) {
		fclose( filep);
		filep = NULL;
	    }
	}
    } else /* MBS_BYTTY */ {

	fflush( m_termout);
	filep = fopen( M_DEVICEOUT, "w");
	if( filep) {
	    FILE *tmp_m_termout;

	    setvbuf( filep, ttybuff, _IOFBF, sizeof(ttybuff));

	    tmp_m_termout = m_termout;
	    m_termout = filep;
	    m_bitcldto( w, h, x, y, d, to, bytesleft);
	    m_termout = tmp_m_termout;
	    /* this mucking about with m_termout might be avoided
	     * by peeking at the definition of m_bitcldto in include/mgr/mgr.h
	    fprintf(filep,"%c%d,%d,%d,%d,%d,%d,%d%c",
		    m_escchar,w,h,x,y,d,to,bytesleft,E_BITLOAD);
	     * which is the worse violation of data hiding?
	     */
	}
    }
    if( filep == NULL)
	busy = MBS_IDLE;
    return( filep? 0: -1);
}


/*
 * Write out some of the bitmap data being transferred.
 * This function may be repeated to do the writing piecemeal,
 * but it must be preceded by m_bitc_sendh and followed by m_bitc_sent.
 * Return 0 for success, -1 for failure.
 */
int m_bitc_sendb( char *data,		/* start of portion of bitmap data */
		  size_t len		/* length in bytes of portion */
		  ) {
    int towr = (len <= bytesleft)? len: bytesleft;
    int writ;

    if( busy != MBS_IDLE) {
	writ = fwrite( data, 1, towr, filep);
	bytesleft -= towr;
    } else
	writ = 0;
    return( writ == towr? 0: -1);
}


/* finalize the transfer of the bitmap and return a success? boolean */
int m_bitc_sent(void) {

    if( busy == MBS_IDLE) {
	errno = EBADF;
	return 0;
    }

    while( bytesleft-- > 0)
	putc( 0, filep);
    fclose( filep);
    if( busy == MBS_BYFILE) {
	int needcopy, wasread, w, h, d;

	needcopy = (offx != 0 || offy != 0);
	m_ttyset();
	m_bitfromfile( needcopy? to_tmp_bm: to_bm, filename);
	m_flush();
	wasread = sscanf( m_get(), "%d %d %d", &w, &h, &d);
	m_ttyreset();
	unlink( filename);
	if( wasread != 3 || w != wide || h != high || d != deep) {
	    errno = EIO;
	    return 0;
	}
	if( needcopy) {
	    m_pushsave( P_BITOP);
	    m_func( BIT_SRC);
	    m_bitcopyto( offx, offy, w, h, 0, 0, to_bm, to_tmp_bm);
	    m_pop();
	    m_bitdestroy( to_tmp_bm);
	}
    }
    busy = MBS_IDLE;
    strcpy( filename+strlen(filename)-6, "XXXXXX");
    return 1;
}
