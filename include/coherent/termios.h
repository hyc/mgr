#include <termio.h>

#define termios termio

#define TCSANOW TCSETA
#define TCSADRAIN TCSETAW
#define TCSAFLUSH TCSETAF

#define TCIOFLUSH TIOCFLUSH
#define TIOCFLUSH ('t'<<8|16)

#define IEXTEN 0

#define tcgetattr(x,y) ioctl(x,TCGETA,y)
#define tcsetattr(x,y,z) ioctl(x,y,z)

#define tcflow(x,y) 0

#define TCOON

#define TCOOFF

#define TIOCNOTTY 0
