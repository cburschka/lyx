// -*- C++ -*-
/**
 * \file QChangesDialog.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS
 */

#ifndef QCHANGESDIALOG_H
#define QCHANGESDIALOG_H

#include "ui/QChangesDialogBase.h"

class QChanges;

class QChangesDialog : public QChangesDialogBase {
	Q_OBJECT
public:
	QChangesDialog(QChanges * form);

protected slots:

	virtual void nextPressed();
	virtual void acceptPressed();
	virtual void rejectPressed();

protected:
	virtual void closeEvent(QCloseEvent * e);

private:
	QChanges * form_;
};

#endif // QCHANGESDIALOG_H
