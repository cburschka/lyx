// -*- C++ -*-
/**
 * \file QErrorDialog.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS
 */

#ifndef QERRORDIALOG_H
#define QERRORDIALOG_H

#ifdef __GNUG__
#pragma interface
#endif

#include "ui/QErrorDialogBase.h"

class QError;

class QErrorDialog : public QErrorDialogBase
{ Q_OBJECT

public:
	QErrorDialog(QError * form);

protected:
	virtual void closeEvent(QCloseEvent * e);

private:
	QError * form_;
};

#endif // QERRORDIALOG_H
