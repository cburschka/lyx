/**
 * \file GUIRunTime.C
 * Copyright 2001 the LyX Team
 * Read the file COPYING
 *
 * \author Jürgen Vigna
 */

#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include "GUIRunTime.h"
#include "debug.h"

#include <kapp.h>

#include FORMS_H_LOCATION

using std::endl;

// tell me again why this test is cluttering up the source rather than
// being in lyxinclude.m4 ? - jbl
#if !defined(FL_REVISION) || FL_REVISION < 88 || FL_VERSION != 0
#error LyX will not compile with this version of XForms.\
       Please get version 0.89.\
       If you want to try to compile anyway, delete this test in src/frontends/kde/GUIRunTime.C.
#endif

extern bool finished;

namespace {

int const xforms_include_version = FL_INCLUDE_VERSION;

} // namespace anon


int GUIRunTime::initApplication(int argc, char * argv[])
{
	// same here, this is clearly a configure-time test - jbl
	int xforms_lib_version = fl_library_version(0, 0);
	if (xforms_include_version != xforms_lib_version) {
		lyxerr << "You are either running LyX with wrong "
			"version of a dynamic XForms library\n"
			"or you have build LyX with conflicting header "
			"and library (different\n"
			"versions of XForms. Sorry but there is no point "
			"in continuing executing LyX!" << endl;
		return 1;
	}

	// I belive this is done in the wrong place, imo this should
	// be done in lyx_gui.C similar to how/where xforms does it (Lgb)
	static KApplication a(argc, argv);

	return 0;
}


void GUIRunTime::processEvents() 
{
	kapp->processEvents();
}


void GUIRunTime::runTime()
{
	// We still use xforms event handler as the main one...
	XEvent ev;
	while (!finished) {
		processEvents();
		if (fl_check_forms() == FL_EVENT) {
			lyxerr << "LyX: This shouldn't happen..." << endl;
			fl_XNextEvent(&ev);
		}
	}
}


void GUIRunTime::setDefaults() 
{
	FL_IOPT cntl;
	cntl.buttonFontSize = FL_NORMAL_SIZE;
	cntl.browserFontSize = FL_NORMAL_SIZE;
	cntl.labelFontSize = FL_NORMAL_SIZE;
	cntl.choiceFontSize = FL_NORMAL_SIZE;
	cntl.inputFontSize = FL_NORMAL_SIZE;
	cntl.menuFontSize  = FL_NORMAL_SIZE;
	cntl.borderWidth = -1;
	cntl.vclass = FL_DefaultVisual;
	fl_set_defaults(FL_PDVisual
			| FL_PDButtonFontSize
			| FL_PDBrowserFontSize
			| FL_PDLabelFontSize
			| FL_PDChoiceFontSize
			| FL_PDInputFontSize
			| FL_PDMenuFontSize
			| FL_PDBorderWidth, &cntl);
}
