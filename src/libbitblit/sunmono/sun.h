#define DATA unsigned long int

#include <mgr/bitblit.h>

extern DATA *graph_mem;

#define LOGBITS 5
#define BITS (~(~(DATA)0<<LOGBITS))

#define bit_linesize(wide,depth) ((((depth)*(wide)+BITS)&~BITS)>>3)

#define BIT_SIZE(m) BIT_Size(BIT_WIDE(m), BIT_HIGH(m), BIT_DEPTH(m))
#define BIT_Size(wide,high,depth) (((((depth)*(wide)+BITS)&~BITS)*(high))>>3)
#define BIT_LINE(x) ((((x)->primary->depth*(x)->primary->wide+BITS)&~BITS)>>LOGBITS)


#ifndef __STDC__
#ifndef __GNUC__
#define	_PROTOTYPE(_function, _params)	_function()
#endif
#endif
#ifndef _PROTOTYPE
#define	_PROTOTYPE(_function, _params)	_function _params
#endif

#include <sys/types.h>
_PROTOTYPE( extern int ioctl, (int _fd, int _request, void *_argp));
_PROTOTYPE( extern int fprintf, (FILE *_stream, const char *_format, ...));
_PROTOTYPE( extern size_t fread, (void *_ptr, size_t _size, size_t _nmemb, FILE *_stream));
_PROTOTYPE( size_t fwrite, (const void *_ptr, size_t _size, size_t _nmemb, FILE *_stream));
_PROTOTYPE( extern int fseek, (FILE *_stream, long _offset, int _whence));
_PROTOTYPE( extern int fflush, (FILE *_stream));
_PROTOTYPE( extern int fclose, (FILE *_stream));
_PROTOTYPE( extern void *memset, (void *_s, int _c, size_t _n));


_PROTOTYPE( extern void display_close, (BITMAP *));

