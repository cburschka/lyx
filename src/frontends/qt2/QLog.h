// -*- C++ -*-
/**
 * \file QLog.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS
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
	QLog();
private:
	/// Apply changes
	virtual void apply() {}
	/// update
	virtual void update_contents();
	/// build the dialog
	virtual void build_dialog();
};

#endif // QLOG_H
