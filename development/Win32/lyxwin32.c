/* This is the wrapper program for LyX on Win32. Using this
 * wrapper program no DOS window will be present running LyX.
 * The bad side of this: no error output could be seen ;-)
 *
 * compile this sourec with following options set:
 *
 *    gcc lyxwin32.c -O2 -o lyxwin32 -static -Wall -Wno-format \
 *                   -Wstrict-prototypes -Wmissing-prototypes \
 *                   -mwindows -e _mainCRTStartup
 *
 * Claus Hentschel, 2002-01-17
 */
#include <stdio.h>            /* standard io library */
#include <stdlib.h>           /* standard library */
#include <unistd.h>           /* sleep , fork & exec */
#include <string.h>           /* standard string library */
#include <errno.h>

int main ( int argc, char *argv[] )
{
	char cmd [32000] = "lyx " ;  /* user command */
	char *nargs [5 ] = {  /* execute with login /bin/bash */
		"/bin/bash", "--login",
		"-c" , cmd ,
		NULL
	} ;
	int i = 1; /* just to count */

	/* ensure bash reads my global env changes */
	putenv ( "BASH_ENV=/etc/lyxprofile" ) ;

	/* do for all "real" args */
	while ( i < argc )
	{
		strcat ( cmd , "\"" ) ; /* add quote before */
		strcat ( cmd , argv [ i ] ) ; /* add the argument */
		strcat ( cmd , "\" " ) ; /* add closing quote */
		i ++ ;
	}

	strcat ( cmd, "</dev/null 2>/tmp/lyx.out");

	fprintf ( stderr , "Command is: |%s|\n" , cmd );
	execv ( "/bin/bash" , nargs ) ;  /* exec sub command */

	/* we should never reach here */
	perror ( "Execute failed") ;
	return ( 0 ) ; /* exit with no error */
}
