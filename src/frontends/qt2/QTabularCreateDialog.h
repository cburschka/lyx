// -*- C++ -*-
/**
 * \file QTabularCreateDialog.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 * \author Edwin Leuven
 *
 * Full author contact details are available in file CREDITS
 */

#ifndef QTABULARCREATEDIALOG_H
#define QTABULARCREATEDIALOG_H

#ifdef __GNUG__
#pragma interface
#endif

#include "ui/QTabularCreateDialogBase.h"

class QTabularCreate;

class QTabularCreateDialog : public QTabularCreateDialogBase
{ Q_OBJECT

public:
	QTabularCreateDialog(QTabularCreate * form);

protected slots:
	virtual void columnsChanged(int);
	virtual void rowsChanged(int);

private:
	QTabularCreate * form_;
};

#endif // QTABULARCREATEDIALOG_H
