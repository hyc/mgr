#include <math.h>

#ifndef OSK
#ifndef PI
#define PI (3.1415926535897932384)
#endif
#endif

double atan2(a,b)
double a, b;
{
    double c;

    if(b==0.0) {
    	return a < 0.0 ? -PI/2 : PI/2;
    }
    c = atan(a/b);
    return a < 0.0 ? (b < 0.0 ? c - PI/2 : c) : (b < 0.0 ? c + PI/2 : c);
}


double cosh(x)
  double x;
{
  return((exp(x) + exp(-x)) / 2.0);
}

double sinh(x)
  double x;
{
  return((exp(x) - exp(-x)) / 2.0);
}

