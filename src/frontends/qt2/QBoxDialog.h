// -*- C++ -*-
/**
 * \file QBoxDialog.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
  * \author Jürgen Spitzmüller
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef QBOXDIALOG_H
#define QBOXDIALOG_H


#include "ui/QBoxDialogBase.h"

class QBox;

class QBoxDialog : public QBoxDialogBase {
	Q_OBJECT
public:
	QBoxDialog(QBox * form);
protected slots:
	virtual void change_adaptor();
	virtual void innerBoxChanged(const QString &);
	virtual void typeChanged(int);
	virtual void restoreClicked();
protected:
	virtual void closeEvent(QCloseEvent * e);
private:
	QBox * form_;
};

#endif // QBOXDIALOG_H
