/* This file is part of
 * ======================================================
 * 
 *           LyX, The Document Processor
 * 	 
 *	    Copyright (C) 1995 Matthias Ettrich
 *          Copyright (C) 1995-1999 The LyX Team.
 *
 *======================================================*/

#include <config.h>
#include FORMS_H_LOCATION
#include "lyx_main.h"
#include "gettext.h"
#include "LString.h"
#include "filetools.h"

// 	$Id: main.C,v 1.1 1999/09/27 18:44:38 larsbj Exp $	

#if !defined(lint) && !defined(WITH_WARNINGS)
static char vcid[] = "$Id: main.C,v 1.1 1999/09/27 18:44:38 larsbj Exp $";
#endif /* lint */

// I keep these here so that it will be processed as early in
// the compilation process as possible.
#if !defined(FL_REVISION) || (FL_REVISION != 88 && FL_REVISION != 89) \
			  || FL_VERSION != 0
#error LyX will not compile with this version of XForms.\
       Please get version 0.89.\
       If you want to try to compile anyway, delete this test in src/main.C.
#endif


int main(int argc, char *argv[]) {
	// lyx_localedir is used by gettext_init() is we have
	//   i18n support built-in
	LString lyx_localedir = getEnvPath("LYX_LOCALEDIR");
	if (lyx_localedir.empty())
		lyx_localedir = LOCALEDIR;
	
	// initialize for internationalized version *EK*
	locale_init();
	gettext_init();

#ifdef __EMX__
	_wildcard(&argc, &argv);
#endif

	LyX lyx(&argc,argv);
	return 0; // ok
}
