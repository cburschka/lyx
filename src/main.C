/* This file is part of
 * ======================================================
 *
 *           LyX, The Document Processor
 *
 *	    Copyright 1995 Matthias Ettrich
 *          Copyright 1995-2001 The LyX Team.
 *
 * ====================================================== */

#include <config.h>

#include "lyx_main.h"
#include "gettext.h"
#include "LString.h"
#include "lyx_gui.h"
#include "support/filetools.h"
#include "support/os.h"
#include "frontends/GUIRunTime.h"


int main(int argc, char * argv[])
{
	os::init(&argc, &argv);
	int const val = GUIRunTime::initApplication(argc, argv);
	if (val)
		return val;

	// lyx_localedir is used by gettext_init() is we have
	//   i18n support built-in
	string lyx_localedir = GetEnvPath("LYX_LOCALEDIR");
	if (lyx_localedir.empty())
		lyx_localedir = LOCALEDIR;

	// initialize for internationalized version *EK*
	locale_init();
	gettext_init(lyx_localedir);

	LyX lyx(&argc, argv);
	return 0; // SUCCESS
}
