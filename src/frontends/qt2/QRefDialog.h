// -*- C++ -*-
/**
 * \file QRefDialog.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Kalle Dalheimer
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS
 */

#ifndef QREFDIALOG_H
#define QREFDIALOG_H

#ifdef __GNUG__
#pragma interface
#endif

#include "ui/QRefDialogBase.h"

class QRef;

class QRefDialog : public QRefDialogBase {
	Q_OBJECT
public:
	QRefDialog(QRef * form);
public slots:
	void changed_adaptor();
	void gotoClicked();
	void refHighlighted(QString const &);
	void refSelected(QString const &);
	void sortToggled(bool);
	void updateClicked();
protected:
	void closeEvent(QCloseEvent * e);
private:
	QRef * form_;
};

#endif // QREFDIALOG_H
