// -*- C++ -*-
/**
 * \file QTabularDialog.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS
 */

#ifndef QTABULARDIALOG_H
#define QTABULARDIALOG_H

#ifdef __GNUG__
#pragma interface
#endif

#include "ui/QTabularDialogBase.h"

class QTabular;

class QTabularDialog : public QTabularDialogBase {
	Q_OBJECT
public:
	QTabularDialog(QTabular * form);

protected slots:
	virtual void change_adaptor();
 
	virtual void columnAppend_clicked();
	virtual void rowAppend_clicked();
	virtual void columnDelete_clicked(); 
	virtual void rowDelete_clicked();
 
protected:
	virtual void closeEvent(QCloseEvent * e);
 
private:
	QTabular * form_;
};

#endif // QTABULARDIALOG_H
