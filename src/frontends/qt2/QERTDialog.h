/**
 * \file QERTDialog.h
 * Copyright 2001 the LyX Team
 * Read the file COPYING
 *
 * \author John Levon <moz@compsoc.man.ac.uk>
 */

#ifndef QERTDIALOG_H
#define QERTDIALOG_H
 
#include <config.h>
 
#include "ui/QERTDialogBase.h"
#include "QERT.h"

class QERTDialog : public QERTDialogBase
{ Q_OBJECT

public:
	QERTDialog(QERT * form);

protected slots:
	void change_adaptor() {
		form_->changed();
	}
 
protected:
	virtual void closeEvent(QCloseEvent * e);

private:
	QERT * form_;
};

#endif // QERTDIALOG_H
