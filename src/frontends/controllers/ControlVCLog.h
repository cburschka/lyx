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

#include "ControlDialogs.h"

/**
 * A controller for the Version Control log viewer.
 */
class ControlVCLog : public ControlDialog<ControlConnectBD> {
public:
	///
	ControlVCLog(LyXView &, Dialogs &);
	///
	string const & logfile() { return logfile_; }

protected:
	///
	virtual void apply() {}
	/// set the params before show or update
	virtual void setParams();
	/// clean-up on hide.
	virtual void clearParams();

private:
	string logfile_;
};

#endif // CONTROLVCLOG_H
