/* This file is part of
 * ====================================================== 
 * 
 *           LyX, The Document Processor
 * 	 
 *	    Copyright 1995 Matthias Ettrich
 *          Copyright 1995-2000 The LyX Team.
 *
 * ====================================================== */

#include <config.h>

#include "lyx_main.h"
#include "gettext.h"
#include "LString.h"
#include "support/filetools.h"
#include "frontends/GUIRunTime.h"


int main(int argc, char * argv[])
{
	int const val = GUIRunTime::initApplication(argc, argv);
	if (val)
		return val;

	// lyx_localedir is used by gettext_init() is we have
	//   i18n support built-in
	string lyx_localedir = GetEnvPath("LYX_LOCALEDIR");
	if (lyx_localedir.empty())
		lyx_localedir = LOCALEDIR;
	
	// initialize for internationalized version *EK*
	locale_init(); // macro
	gettext_init(); // macro

#ifdef __EMX__
	_wildcard(&argc, &argv);
#endif

	LyX lyx(&argc, argv);
	return 0; // SUCCESS
}
