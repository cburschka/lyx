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

#ifdef __GNUG__
#pragma interface
#endif

#include "ControlDialog_impl.h"
#include "Lsstream.h"

/**
 * A controller for the Version Control log viewer.
 */
class ControlVCLog : public ControlDialogBD {
public:
	///
	ControlVCLog(LyXView &, Dialogs &);
	/// get a stringstream containing the log file
	stringstream & getVCLogFile(stringstream & ss) const;
	/// get the filename of the buffer
	string const getBufferFileName() const;

private:
	///
	virtual void apply() {}
};

#endif // CONTROLVCLOG_H
