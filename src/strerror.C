/*****************************************************
 * provides strerror()
 * author Stephan Witt <stephan.witt@beusen.de>
 *****************************************************/

#include <config.h>

// 	$Id: strerror.C,v 1.1 1999/09/27 18:44:38 larsbj Exp $	

#if !defined(lint) && !defined(WITH_WARNINGS)
static char vcid[] = "$Id: strerror.C,v 1.1 1999/09/27 18:44:38 larsbj Exp $";
#endif /* lint */

extern	int 	sys_nerr ;
extern	char *	sys_errlist [] ;

char * strerror (int errnum)
{
	static	char *	errtext = "unknown errno" ;

	if ( errnum < sys_nerr )
		return sys_errlist [errnum] ;
	return errtext ;
}
