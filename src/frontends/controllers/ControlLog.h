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
 * \file ControlLog.h
 * \author John Levon, moz@compsoc.man.ac.uk
 * \author Angus Leeming <a.leeming@ic.ac.uk>
 */

#ifndef CONTROLLOG_H
#define CONTROLLOG_H

#include <utility>

#ifdef __GNUG__
#pragma interface
#endif

#include "ControlConnections.h"
#include "buffer.h" // Buffer::LogType

/**
 * A controller for a read-only text browser.
 */
class ControlLog : public ControlConnectBD {
public:
	///
	ControlLog(LyXView &, Dialogs &);
	///
	std::pair<Buffer::LogType, string> const & logfile()
		{ return logfile_; }

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
	std::pair<Buffer::LogType, string> logfile_;
};


/** A class to instantiate and make available the GUI-specific
    ButtonController and View.
 */
template <class GUIview, class GUIbc>
class GUILog : public ControlLog {
public:
	///
	GUILog(LyXView &, Dialogs &);
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
GUILog<GUIview, GUIbc>::GUILog(LyXView & lv, Dialogs & d)
	: ControlLog(lv, d),
	  view_(*this)
{}

#endif // CONTROLLOG_H
