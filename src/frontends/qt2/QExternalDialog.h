// -*- C++ -*-
/**
 * \file QExternalDialog.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS
 */

#ifndef QEXTERNALDIALOG_H
#define QEXTERNALDIALOG_H

#ifdef __GNUG__
#pragma interface
#endif

#include "ui/QExternalDialogBase.h"

class QExternal;

class QExternalDialog : public QExternalDialogBase {
	Q_OBJECT
public:
	QExternalDialog(QExternal * form);

	virtual void show();
protected slots:
	virtual void change_adaptor();
	virtual void editClicked();
	virtual void viewClicked();
	virtual void updateClicked();
	virtual void browseClicked();
	virtual void templateChanged();
protected:
	virtual void closeEvent(QCloseEvent * e);
private:
	QExternal * form_;
};

#endif // QEXTERNALDIALOG_H
