// -*- C++ -*-
/**
 * \file QIndexDialog.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS
 */

#ifndef QINDEXDIALOG_H
#define QINDEXDIALOG_H

#ifdef __GNUG__
#pragma interface
#endif

#include "ui/QIndexDialogBase.h"

class QIndex;

class QIndexDialog : public QIndexDialogBase
{ Q_OBJECT

public:
	QIndexDialog(QIndex * form);

	virtual void show();

protected slots:
	virtual void change_adaptor();

protected:
	virtual void closeEvent(QCloseEvent * e);

private:
	QIndex * form_;
};

#endif // QINDEXDIALOG_H
