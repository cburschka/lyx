// -*- C++ -*-
/* This file is part of
 * ======================================================
 * 
 *           LyX, The Document Processor
 *
 *           Copyright 2000 The LyX Team.
 *
 * ====================================================== */

#ifndef GUIRUNTIME_H
#define GUIRUNTIME_H

#ifdef __GNUG__
#pragma interface
#endif

class LyXView;

/** The LyX GUI independent guiruntime class
    The GUI interface is implemented in the corresponding
    frontends GUIRunTime.C file.
*/
class GUIRunTime {
public:
	///
	static
	void processEvents();
	///
	static
	void runTime();
};
#endif
