// -*- C++ -*-
/**
 * \file QBibitem.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef QBIBITEM_H
#define QBIBITEM_H


#include "QDialogView.h"

class ControlCommand;
class QBibitemDialog;

class QBibitem
	: public QController<ControlCommand, QView<QBibitemDialog> >
{
public:
	friend class QBibitemDialog;

	QBibitem(Dialog &);
protected:
	virtual bool isValid();
private:
	/// Apply changes
	virtual void apply();
	/// update
	virtual void update_contents();
	/// build the dialog
	virtual void build_dialog();
};

#endif // QBIBITEM_H
