// -*- C++ -*-
/**
 * \file QERTDialog.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS
 */

#ifndef QERTDIALOG_H
#define QERTDIALOG_H

#include "ui/QERTDialogBase.h"

#ifdef __GNUG__
#pragma interface
#endif

class QERT;

class QERTDialog : public QERTDialogBase {
	Q_OBJECT
public:
	QERTDialog(QERT * form);
protected slots:
	virtual void change_adaptor();
protected:
	virtual void closeEvent(QCloseEvent * e);
private:
	QERT * form_;
};

#endif // QERTDIALOG_H
