// -*- C++ -*-
/**
 * \file QIndex.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 * \author Kalle Dalheimer
 *
 * Full author contact details are available in file CREDITS
 */

#ifndef QINDEX_H
#define QINDEX_H


#include "QDialogView.h"

class ControlCommand;
class QIndexDialog;


class QIndex :
	public QController<ControlCommand, QView<QIndexDialog> >
{
public:
	friend class QIndexDialog;

	QIndex(Dialog &, QString const & title, QString const & label);
protected:
	virtual bool isValid();
private:
	/// Apply changes
	virtual void apply();
	/// update
	virtual void update_contents();
	/// build the dialog
	virtual void build_dialog();

	///
	QString const label_;
};

#endif // QINDEX_H
