// -*- C++ -*-
/* This file is part of
 * ====================================================== 
 *
 *           LyX, The Document Processor
 *
 *           Copyright 2001 The LyX Team.
 *
 * ======================================================
 *
 * \file ControlVCLog.h
 * \author John Levon, moz@compsoc.man.ac.uk
 * \author Angus Leeming <a.leeming@ic.ac.uk>
 */

#ifndef CONTROLVCLOG_H
#define CONTROLVCLOG_H

#include <utility>

#ifdef __GNUG__
#pragma interface
#endif

#include "ControlConnections.h"

/**
 * A controller for the Version Control log viewer.
 */
class ControlVCLog : public ControlConnectBD {
public:
	///
	ControlVCLog(LyXView &, Dialogs &);
	///
	string const & logfile() { return logfile_; }

protected:
	///
	virtual void apply() {}
	/// Show the dialog.
	virtual void show();
	/// Update the dialog.
	virtual void update();
	/// Hide the dialog.
	virtual void hide();

private:
	string logfile_;
};


/** A class to instantiate and make available the GUI-specific
    ButtonController and View.
 */
template <class GUIview, class GUIbc>
class GUIVCLog : public ControlVCLog {
public:
	///
	GUIVCLog(LyXView &, Dialogs &);
	///
	virtual ButtonControllerBase & bc() { return bc_; }
	///
	virtual ViewBase & view() { return view_; }

private:
	///
	ButtonController<OkCancelPolicy, GUIbc> bc_;
	///
	GUIview view_;
};

template <class GUIview, class GUIbc>
GUIVCLog<GUIview, GUIbc>::GUIVCLog(LyXView & lv, Dialogs & d)
	: ControlVCLog(lv, d),
	  view_(*this)
{}

#endif // CONTROLVCLOG_H
