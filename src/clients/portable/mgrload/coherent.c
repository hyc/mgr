/* COHERENT.c query the running total of system times in the proc chain
 * display results graphically - uload 100 = load 1, 110 = load 2, 120 = load 3
 * for uload<100, use 100-%idle for load; thus 70%idle = 0.3 load.
 *
 * Based on dtime.c by Randy Wright (rw@rwsys.wimsey.bc.ca) and uses his mz 
 * driver.  By Harry C. Pulley, IV (hcp@csx.cciw.ca), 1993
 */

#include <stdio.h>
#include <sys/mzioctl.h>
#include <sys/fcntl.h>

#define INTERVAL 5

extern FILE *memfile;

double aveload=0.0;

double getload()
{
	static mzattr_t zstime, oldstime=0, sval, zutime, oldutime=0, uval, val;
	int i;
	double load, err;

	do {
        	/* ioctl function 1, get sum of system times */
        	i = ioctl( (fileno(memfile)), 2, &zstime );

		/* ioctl function 1, get sum of user times */
		i = ioctl( (fileno(memfile)), 3, &zutime );
	
        	/* zstime advances HZ per sec if there is no load */
        	sval = 101 - ((zstime - oldstime)/ INTERVAL);
	
		/* zutime advances HZ per sec if system is fully loaded */
		uval =  ((zutime - oldutime)/ INTERVAL);
	
        	oldstime = zstime;
		oldutime = zutime;
	
	/* if one of these values is negative don't printf or wait
 	* just get the values again.
 	* Negative values seems to happen when a PROC is unlinked or
 	* one PROC is substituted for another.
 	*/
	} while ( sval < 0 || uval < 0 );

	val=(uval+sval)/2;

	if (val>99)
		load=((double)(val-100))/10.0;
	else
		load=((double)val)/100.0;

#ifdef DEBUG
	printf("load %lf sval %d uval %d\n",load,sval,uval);
#endif

	err=load-aveload;

	aveload+=err/(60/INTERVAL);	

	return aveload;
}
/* end of COHERENT.c*/
