// -*- C++ -*-
/**
 * \file QSendtoDialog.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Juergen Spitzmueller
 *
 * Full author contact details are available in file CREDITS
 */

#ifndef QSENDTODIALOG_H
#define QSENDTODIALOG_H

#ifdef __GNUG__
#pragma interface
#endif

#include "ui/QSendtoDialogBase.h"

class QSendto;

class QSendtoDialog : public QSendtoDialogBase {
	Q_OBJECT
public:
	QSendtoDialog(QSendto * form);
protected slots:
	virtual void changed_adaptor();
	virtual void slotFormatHighlighted(const QString&) {}
	virtual void slotFormatSelected(const QString&) {}
protected:
	virtual void closeEvent(QCloseEvent * e);
private:
	QSendto * form_;
};

#endif // QSENDTODIALOG_H
