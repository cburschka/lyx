// -*- C++ -*-
/**
 * \file QTabularCreate.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS
 */

#ifndef QTABULARCREATE_H
#define QTABULARCREATE_H


#include "QDialogView.h"


class ControlTabularCreate;
class QTabularCreateDialog;


///
class QTabularCreate
	: public QController<ControlTabularCreate, QView<QTabularCreateDialog> >
{
public:
	///
	friend class QTabularCreateDialog;
	///
	QTabularCreate(Dialog &);
private:
	/// Apply changes
	virtual void apply();
	/// update
	virtual void update_contents() {}
	/// build the dialog
	virtual void build_dialog();
};

#endif // QTABULARCREATE_H
