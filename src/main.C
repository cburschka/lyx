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
#include <iostream>

#ifdef KDEGUI
#    include <kapp.h>
#endif
#ifdef GTKGUI
#    include <gnome--/main.h>
#    include "frontends/gnome/mainapp.h"
GLyxAppWin * mainAppWin;
#endif

#include FORMS_H_LOCATION

#include "lyx_main.h"
#include "gettext.h"
#include "LString.h"
#include "support/filetools.h"


// I keep these here so that it will be processed as early in
// the compilation process as possible.
#if !defined(FL_REVISION) || FL_REVISION < 88 || FL_VERSION != 0
#error LyX will not compile with this version of XForms.\
       Please get version 0.89.\
       If you want to try to compile anyway, delete this test in src/main.C.
#endif

using std::cerr;
using std::endl;

static int const xforms_include_version = FL_INCLUDE_VERSION;

int main(int argc, char * argv[])
{
	// Check the XForms version in the forms.h header against
	// the one in the libforms. If they don't match quit the
	// execution of LyX. Better with a clean fast exit than
	// a strange segfault later.
	// I realize that this check have to be moved when we
	// support several toolkits, but IMO all the toolkits
	// should try to have the same kind of check. This could
	// be done by having a CheckHeaderAndLib function in
	// all the toolkit implementations, this function is
	// responsible for notifing the user.
	// if (!CheckHeaderAndLib()) {
	//         // header vs. lib version failed
	//         return 1;
	// }
	int xforms_lib_version = fl_library_version(0, 0);
	if (xforms_include_version != xforms_lib_version) {
		cerr << "You are either running LyX with wrong "
			"version of a dynamic XForms library\n"
			"or you have build LyX with conflicting header "
			"and library (different\n"
			"versions of XForms. Sorry but there is no point "
			"in continuing executing LyX!" << endl;
		return 1;
	}

#ifdef KDEGUI
	KApplication a( argc, argv );
#endif
#ifdef GTKGUI
	string app_id(PACKAGE);
	string app_version(VERSION);
	Gnome::Main  a(app_id, app_version, 1, argv); //argc, argv );
	GLyxAppWin   appWin;
	mainAppWin = &appWin;
#endif
	
	// lyx_localedir is used by gettext_init() is we have
	//   i18n support built-in
	string lyx_localedir = GetEnvPath("LYX_LOCALEDIR");
	if (lyx_localedir.empty())
		lyx_localedir = LOCALEDIR;
	
	// initialize for internationalized version *EK*
	locale_init();
	gettext_init();

#ifdef __EMX__
	_wildcard(&argc, &argv);
#endif

	LyX lyx(&argc, argv);
	return 0; // SUCCESS
}
