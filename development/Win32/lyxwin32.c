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
 *
 * ================================================================
 * Enhanced to pass filename argument(s) to LyX. Now lyxwin32.exe
 * can be used to associate all lyx-files with LyX ;-)
 *
 * Claus Hentschel, 2002-07-04
 */
#include <stdio.h>            /* standard io library */
#include <stdlib.h>           /* standard library */
#include <unistd.h>           /* sleep , fork & exec */
#include <string.h>           /* standard string library */
#include <errno.h>
#include <sys/cygwin.h>

char *internal_path(char *p, char *pp);
	
int main ( int argc, char *argv[] )
{
	char cmd [32000] = "lyx " ;  /* user command */
	char *nargs [5] = {  /* execute with login /bin/bash */
		"/bin/bash", "--login",
		"-c" , cmd ,
		NULL
	} ;
	int i = 1; /* just to count */
	char posixpath[1024]; /* POSIX style pathname if filename is passed */

	/* ensure bash reads my global env changes */
	putenv ( "BASH_ENV=/etc/lyxprofile" ) ;

	/* do for all "real" args */
	while ( i < argc )
	{
		strcat ( cmd , "\"" ) ; /* add quote before */
		if (strcmp(internal_path(argv[i], posixpath), argv[i]) == 0)
			strcat ( cmd , argv [ i ] ) ; /* add the argument */
		else
			strcat ( cmd , posixpath ) ; /* add the filename */
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

/* returns converted name just for an easier usage ;-)
 */
char *internal_path(char *p, char *pp) {
	cygwin_conv_to_posix_path(p, pp);
	return pp;
}
