// -*- C++ -*-
/**
 * \file QVCLog.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS
 */

#ifndef QVCLOG_H
#define QVCLOG_H


#include "QDialogView.h"


class ControlVCLog;
class QVCLogDialog;

///
class QVCLog
	: public QController<ControlVCLog, QView<QVCLogDialog> >
{
public:
	///
	friend class QVCLogDialog;
	///
	QVCLog(Dialog &);
private:
	/// Apply changes
	virtual void apply() {}
	/// update
	virtual void update_contents();
	/// build the dialog
	virtual void build_dialog();
};

#endif // QVCLOG_H
