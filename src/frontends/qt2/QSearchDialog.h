/**
 * \file QSearchDialog.h
 * Copyright 2001 the LyX Team
 * Read the file COPYING
 *
 * \author Edwin Leuven
 */

#ifndef QSEARCHDIALOG_H
#define QSEARCHDIALOG_H

#include <config.h>
 
#include "ui/QSearchDialogBase.h"
#include "QSearch.h"

class QCloseEvent;

class QSearchDialog : public QSearchDialogBase
{ Q_OBJECT

public:
	QSearchDialog(QSearch * form, QWidget * parent = 0, const char * name = 0, bool modal = FALSE, WFlags fl = 0);
	~QSearchDialog();
	
	void setReadOnly(bool);
	
	void Replace(bool replaceall = false);
 
protected:
	void closeEvent(QCloseEvent * e);

private:
	QSearch * form_;

protected slots:

	void Find();
 
	void Replace() {
		Replace(false);
	};
 
	void ReplaceAll() {
		Replace(true);
	};

	void cancel_adaptor() {
		form_->close();
		hide();
	}
};

#endif // QSEARCHDIALOG_H
