// -*- C++ -*-
/**
 * \file QURLDialog.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS
 */

#ifndef QURLDIALOG_H
#define QURLDIALOG_H

#ifdef __GNUG__
#pragma interface
#endif

#include "ui/QURLDialogBase.h"

class QURL;

class QURLDialog : public QURLDialogBase
{ Q_OBJECT

public:
	QURLDialog(QURL * form);
	~QURLDialog();

	virtual void show();

public slots:
	void changed_adaptor();

protected:
	void closeEvent(QCloseEvent *);

private:
	QURL * form_;
};

#endif // QURLDIALOG_H
