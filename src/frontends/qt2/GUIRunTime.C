/**
 * \file GUIRunTime.C
 * Copyright 2001 the LyX Team
 * Read the file COPYING
 *
 * \author John Levon
 */

#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include <qapplication.h>
#include <qpainter.h>
 
#include "QtLyXView.h" 
#include "XFormsView.h"
#include "GUIRunTime.h"
#include "debug.h"

#include FORMS_H_LOCATION

using std::endl;

// For now we need this here as long as we use xforms components!

// I keep these here so that it will be processed as early in
// the compilation process as possible.
#if !defined(FL_REVISION) || FL_REVISION < 88 || FL_VERSION != 0
#error LyX will not compile with this version of XForms.\
       Please get version 0.89.\
       If you want to try to compile anyway, delete this test in src/frontends/qt2/GUIRunTime.C.
#endif

extern bool finished;

int GUIRunTime::initApplication(int & argc, char * argv[])
{
	int const xforms_include_version = FL_INCLUDE_VERSION;
 
	// Check the XForms version in the forms.h header against
	// the one in the libforms. If they don't match quit the
	// execution of LyX. Better with a clean fast exit than
	// a strange segfault later.
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
	static QApplication a(argc, argv);
	return 0;
}

 
void GUIRunTime::processEvents() 
{
	qApp->processEvents();
}


void GUIRunTime::runTime()
{
	// We still use xforms event handler as the main one...
	XEvent ev;
	while (!finished) {
		processEvents();
		if (fl_check_forms() == FL_EVENT) {
			fl_XNextEvent(&ev);
			lyxerr << "Received unhandled X11 event" << endl;
			lyxerr << "Type: 0x" << std::hex << ev.xany.type <<
				"Target: 0x" << std::hex << ev.xany.window << endl;
		}
	}
}


LyXView * GUIRunTime::createMainView(int w, int h)
{
	return new XFormsView(w, h);
}

 
Display * GUIRunTime::x11Display()
{
	//return p.device()->x11Display();
	return fl_get_display();
}


int GUIRunTime::x11Screen()
{
	//return p.device()->x11Screen(); 
	return fl_screen;
}


Colormap GUIRunTime::x11Colormap()
{
	//return p.device()->x11Colormap(); 
	return fl_state[fl_get_vclass()].colormap;
}

 
int GUIRunTime::x11VisualDepth() 
{
	//return p.device()->x11Depth();
	return fl_get_visual_depth();
}

 
float GUIRunTime::getScreenDPI()
{
	Screen * scr = ScreenOfDisplay(fl_get_display(), fl_screen);
	return ((HeightOfScreen(scr) * 25.4 / HeightMMOfScreen(scr)) +
		(WidthOfScreen(scr) * 25.4 / WidthMMOfScreen(scr))) / 2;
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
