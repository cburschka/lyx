// -*- C++ -*-
/**
 * \file QVCLog.h
 * Copyright 2001 the LyX Team
 * Read the file COPYING
 *
 * \author John Levon <moz@compsoc.man.ac.uk>
 */

#ifndef QVCLOG_H
#define QVCLOG_H

#ifdef __GNUG__
#pragma interface
#endif

#include "Qt2Base.h"

class ControlVCLog;
class QVCLogDialog;

///
class QVCLog
	: public Qt2CB<ControlVCLog, Qt2DB<QVCLogDialog> >
{
public:
	///
	friend class QVCLogDialog;
	///
	QVCLog();

private:
	/// Apply changes
	virtual void apply() {};
	/// update
	virtual void update_contents();
	/// build the dialog
	virtual void build_dialog();
};

#endif // QVCLOG_H
