/**
 * \file lyxeditor.c
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Enrico Forestieri
 *
 * Full author contact details are available in file CREDITS.
 *
 * This is a wrapper program for the lyxeditor.sh script or lyxclient program,
 * meant to be used with yap or sumatrapdf for inverse search.
 */

#include "config.h"
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <cygwin/version.h>
#include <sys/cygwin.h>
#include <windows.h>

void convert_to_full_posix_path(char const * from, char *to)
{
#if CYGWIN_VERSION_DLL_MAJOR >= 1007
    cygwin_conv_path(CCP_WIN_A_TO_POSIX, from, to, PATH_MAX);
#else
    cygwin_conv_to_full_posix_path(from, to);
#endif
}

int main(int ac, char **av)
{
	char * buf;
	int bufsize;
	char posixpath[PATH_MAX];

	if (ac < 3 || ac > 4) {
		MessageBox(0, "Usage: lyxeditor [-g] <file.tex> <lineno>",
				"ERROR: Wrong number of arguments", 0);
		return 1;
	}

	if (ac == 3) {
		char const * fmt = "lyxeditor.sh" PROGRAM_SUFFIX " '%s' %s";
		convert_to_full_posix_path(av[1], posixpath);
		bufsize = snprintf(0, 0, fmt, posixpath, av[2]) + 1;
		if ((buf = malloc(bufsize)))
			snprintf(buf, bufsize, fmt, posixpath, av[2]);
	} else {
		char const * fmt = "lyxclient" PROGRAM_SUFFIX " %s '%s' %s";
		convert_to_full_posix_path(av[2], posixpath);
		bufsize = snprintf(0, 0, fmt, av[1], posixpath, av[3]) + 1;
		if ((buf = malloc(bufsize)))
			snprintf(buf, bufsize, fmt, av[1], posixpath, av[3]);
	}

	if (!buf) {
		MessageBox(0, "Too long arguments", "lyxeditor", 0);
		return 1;
	}
	system(buf);
	free(buf);
	return 0;
}
