#include <sys/types.h>
#include <stdio.h>

#include "types.h"

#ifdef __STDC__

/* linux wants size_t not int
extern void bcopy( const void *, void *, int);
extern void bzero( void *, int);
 */
extern int fclose( FILE *);
extern int fflush( FILE *);
extern int fprintf( FILE *, const char *, ...);
extern int fputc( int, FILE *);
extern size_t fread( void *, size_t, size_t, FILE *);
extern void free( void *);
extern int fscanf( FILE *stream, const char *format, ...);
extern int fseek( FILE *, long int, int);
extern size_t fwrite( const void *, size_t, size_t, FILE *);
extern int getopt( int argc, char *const *argv, const char *shortopts);
/* extern int open( const char *pathname, int flags, ...); */
extern void perror( const char *);
extern void rewind( FILE *);
/* extern int sprintf( char *, const char *, ...); */
extern int sscanf( const char *, const char *, ...);

extern int FindPostAmble( register FILE *f);
extern void GripeCannotFindPostamble( void);
extern void GripeCannotGetFont( char *name, i32 mag, i32 dsz, char *dev,
				char *fullname);
extern void GripeDifferentChecksums( char *font, i32 tfmsum, i32 fontsum);
extern void GripeMismatchedValue( char *s);
extern void GripeMissingFontsPreventOutput( int n);
extern void GripeMissingOp( char *s);
extern void GripeMissingOp( char *s);
extern void GripeNoSuchFont( i32 n);
extern void GripeUndefinedOp( int n);
extern void GripeUnexpectedOp( char *s);
extern int ScanPostAmble( register FILE *f,
			  void( *headerfunc) (/* ??? */),
			  void( *fontfunc) (/* ??? */));
extern double DMagFactor( int mag);
extern int split( register char *s, register char **w, int nw);

extern void error( );	/* error and panic have varargs */
extern void panic( );

#endif
