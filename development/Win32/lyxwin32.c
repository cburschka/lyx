#include <stdio.h>            /* standard io library */
#include <stdlib.h>           /* standard library */
#include <unistd.h>           /* sleep , fork & exec */
#include <string.h>           /* standard string library */
#include <errno.h>

int main ( int argc, char *argv[] )
{
	char cmd [32000] = "lyx " ;  /* user command */
	char *nargs [4 ] = {  /* execute with login /bin/bash */
		"/bin/bash",
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

	strcat ( cmd, "</dev/null 2>/dev/null");

	fprintf ( stderr , "Command is: |%s|\n" , cmd );
	execv ( "/bin/bash" , nargs ) ;  /* exec sub command */

	/* Oops: we should never reach here */
	perror ( "Execute failed") ;
	return ( 1 ) ; /* exit with an error */
}
