// -*- C++ -*-
/* This file is part of
 * ======================================================
 * 
 *           LyX, The Document Processor
 *
 *           Copyright 2000-2001 The LyX Team.
 *
 * ====================================================== */

#ifndef GUIRUNTIME_H
#define GUIRUNTIME_H

#ifdef __GNUG__
#pragma interface
#endif

#include <X11/Xlib.h>
class LyXView;

/** The LyX GUI independent guiruntime class
    The GUI interface is implemented in the corresponding
    frontends GUIRunTime.C file.
*/
class GUIRunTime {
public:
	/// initialise the toolkit 
	static
	int initApplication(int argc, char * argv[]);
	/// process pending events
	static
	void processEvents();
	/// enter the permanent event loop until "finished" becomes false
	static
	void runTime();
	/// This is run first in the LyXGUI constructor.
	static
	void setDefaults();
	///
	static
	LyXView * createMainView(int w, int h);

/* the following entries are X11 specific and should eventually go away */
	/// The display on which LyX is running
	static
	Display * x11Display();
	/// The screen on which LyX is running
	static
	int x11Screen();
	/// The current colormap
	static
	Colormap x11Colormap();
	/// The current visual depth
	static
	int x11VisualDepth();
};
#endif
