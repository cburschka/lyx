/**
 * \file main.C
 * Copyright 2002 the LyX Team
 * Read the file COPYING
 *
 * \author unknown
 */

#include <config.h>

#include "lyx_main.h"
#include "gettext.h"
#include "LString.h"
#include "support/filetools.h"
#include "support/os.h"
 
int main(int argc, char * argv[])
{
	os::init(&argc, &argv);

	// lyx_localedir is used by gettext_init() is we have
	//   i18n support built-in
	string lyx_localedir = GetEnvPath("LYX_LOCALEDIR");
	if (lyx_localedir.empty())
		lyx_localedir = LOCALEDIR;

	// initialize for internationalized version *EK*
	locale_init();
	gettext_init(lyx_localedir);

	LyX lyx(argc, argv);
	return 0;
}
