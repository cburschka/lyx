// -*- C++ -*-
/**
 * \file QLog.h
 * Copyright 2001 the LyX Team
 * Read the file COPYING
 *
 * \author John Levon <moz@compsoc.man.ac.uk>
 */

#ifndef QLOG_H
#define QLOG_H

#ifdef __GNUG__
#pragma interface
#endif

#include "Qt2Base.h"

class ControlLog;
class QLogDialog;

///
class QLog
	: public Qt2CB<ControlLog, Qt2DB<QLogDialog> > 
{
public:
	///
	friend class QLogDialog;
	///
	QLog(ControlLog &);

private:
	/// Apply changes
	virtual void apply() {};
	/// update
	virtual void update_contents();
	/// build the dialog
	virtual void build_dialog();
};

#endif // QLOG_H
