/**
 * \file lyxwin.c
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Claus Hentschel
 * \author Enrico Forestieri
 *
 * Full author contact details are available in file CREDITS.
 *
 * This is the wrapper program for LyX/Cygwin. Using this wrapper no
 * DOS window will be present when running LyX from the Windows GUI.
 * The bad side of this: no error output can be seen ;-)
 *
 * It launches the real binary using the native Windows GUI.
 */

#include "config.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <process.h>
#include <string.h>
#include <limits.h>
#include <cygwin/version.h>
#include <sys/cygwin.h>
#include <windows.h>

void convert_to_posix_path(char const * from, char *to)
{
#if CYGWIN_VERSION_DLL_MAJOR >= 1007
    cygwin_conv_path(CCP_WIN_A_TO_POSIX | CCP_RELATIVE, from, to, PATH_MAX);
#else
    cygwin_conv_to_posix_path(from, to);
#endif
}

int main (int argc, char **argv, char **environ)
{
	FILE *fp;
	char *s;
	char posixpath[PATH_MAX];
	char cmd[4096] = PACKAGE " ";
	char const *nargs[5] = {
		"/bin/bash", "--login",
		"-c", cmd,
		NULL
	};
	int i = 1;

	while (i < argc) {
		int done = 0;
		int lyxfile = (s = strrchr(argv[i], '.'))
				&& strcasecmp(s, ".lyx") == 0;
		/* Add initial quote */
		strcat(cmd, "\"");
		convert_to_posix_path(argv[i], posixpath) ;
		/* Hack to account for shares */
		if (lyxfile && argv[i][0] == '\\' && argv[i][1] != '\\')
			strcat(cmd, "/");
		/* add the argument */
		strcat(cmd, posixpath);
		/* add closing quote */
		strcat(cmd, "\" ");
		if (!done && lyxfile && (s = strrchr(posixpath,'/'))) {
			*s = '\0';
			if (setenv("CDPATH", posixpath, 1) == 0)
				done = 1;
		}
		++i;
	}

	strcat(cmd, "</dev/null 1>/dev/null 2>&1");

	/* fprintf(stderr , "Command is: |%s|\n", cmd); */
	/* ensure bash reads our global env changes */
	putenv("BASH_ENV=" LYX_ABS_INSTALLED_DATADIR "/lyxprofile") ;
	/* exec sub command */
	spawnv(_P_NOWAIT, "/bin/bash", nargs);
	/* exit with no error */
	return(0) ;
}
