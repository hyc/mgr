/*{{{}}}*/
/*{{{  Notes*/
/**
	Given a bitmap id and an icon name,
	have mgr load that icon into that bitmap, returning the icon width
	and height via the given integer pointers.
	Return a positive number if successful.
	If the icon is not loaded, set the width and height values to 0 and
	return 0.
*/
/*}}}  */
/*{{{  #includes*/
#include <mgr/mgr.h>
#include <stdio.h>
/*}}}  */

/*{{{  m_bitfile*/
int m_bitfile(int bitmapid, char *iconname, int *iconwidthp, int *iconheightp, int *icondepthp)
{
	*iconwidthp = *iconheightp = *icondepthp = 0;
	m_bitfromfile( bitmapid, iconname );
	m_flush();
	return(sscanf(m_get(),"%d %d %d",iconwidthp,iconheightp,icondepthp)==3);
}
/*}}}  */
