/**
 * \file QURLDialog.h
 * Copyright 2001 the LyX Team
 * Read the file COPYING
 *
 * \author John Levon
 */

#ifndef QURLDIALOG_H
#define QURLDIALOG_H
 
#include "ui/QURLDialogBase.h"

class QURL;

class QURLDialog : public QURLDialogBase
{ Q_OBJECT

public:
	QURLDialog(QURL * form, QWidget * parent = 0, const char * name = 0, bool modal = FALSE, WFlags fl = 0);
	~QURLDialog();

public slots:
	void apply_adaptor();
	void close_adaptor();

protected:
	void closeEvent(QCloseEvent *);

private:
	QURL * form_;
};

#endif // QURLDIALOG_H
