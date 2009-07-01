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
#include <sys/cygwin.h>
#include <windows.h>

int main(int ac, char **av)
{
	char buf[2 * PATH_MAX];
	char posixpath[PATH_MAX + 1];

	if (ac < 3 || ac > 4) {
		MessageBox(0, "Usage: lyxeditor [-g] <file.tex> <lineno>",
				"ERROR: Wrong number of arguments", 0);
		return 1;
	}

	if (ac == 3) {
		cygwin_conv_to_full_posix_path(av[1], posixpath);
		sprintf(buf, "lyxeditor.sh" PROGRAM_SUFFIX " '%s' %s",
				posixpath, av[2]);
	} else {
		cygwin_conv_to_full_posix_path(av[2], posixpath);
		sprintf(buf, "lyxclient" PROGRAM_SUFFIX " %s '%s' %s",
				av[1], posixpath, av[3]);
	}
	system(buf);
	return 0;
}
