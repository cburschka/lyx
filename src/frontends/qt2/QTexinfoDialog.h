// -*- C++ -*-
/**
 * \file QTexinfoDialog.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Edwin Leuven
 *
 * Full author contact details are available in file CREDITS
 */

#ifndef QTEXINFODIALOG_H
#define QTEXINFODIALOG_H

#ifdef __GNUG__
#pragma interface
#endif

#include "ui/QTexinfoDialogBase.h"

class QTexinfo;

class QTexinfoDialog : public QTexinfoDialogBase
{ Q_OBJECT

public:
	QTexinfoDialog(QTexinfo * form);

protected slots:
	virtual void change_adaptor();
	virtual void rescanClicked();
	virtual void viewClicked();
	virtual void update();
	virtual void enableViewPB();

protected:
	virtual void closeEvent(QCloseEvent * e);
private:
	QTexinfo * form_;
};

#endif // QTEXINFODIALOG_H
