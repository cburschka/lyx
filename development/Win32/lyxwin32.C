#include <stdio.h>                         /* standard io library */
#include <stdlib.h>                        /* standard library */
#include <unistd.h>                        /* sleep , fork & exec */
#include <string.h>                        /* standard string library */
#include <errno.h>

int main ( int argc, char *argv[] )
{
/*char cmd [32000] = "lyx " ; */                   /* user command */
char cmd [32000] = "lyx " ;
char *nargs [4 ] = { "/bin/bash", "-c" , cmd , NULL } ;   /* execute with login /bin/bash */
int i=1;

putenv ( "BASH_ENV=/etc/lyxprofile" ) ;    /* ensure bash reads my global env changes */

while ( i < argc )                      /* do for all "real" args */
   {
       strcat ( cmd , "\"" ) ;             /* add quote before */
       strcat ( cmd , argv [ i ] ) ;       /* add the argument */
       strcat ( cmd , "\" " ) ;            /* add closing quote */
       i ++ ;
   }

strcat ( cmd, "</dev/null 2>/dev/null");

fprintf ( stderr , "Command is: |%s|\n" , cmd );
execv ( "/bin/bash" , nargs ) ;         /* exec sub command */

/* we should never reach here */
fprintf ( stderr , "Execute failed, error = %d\n" , errno ) ;
return ( 0 ) ;                          /* exit with no error */
}
