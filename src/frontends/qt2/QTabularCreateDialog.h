/**
 * \file QTabularCreateDialog.h
 * Copyright 2001 the LyX Team
 * Read the file COPYING
 *
 * \author John Levon <moz@compsoc.man.ac.uk>
 * \author Edwin Leuven <leuven@fee.uva.nl>
 */

#ifndef QTABULARCREATEDIALOG_H
#define QTABULARCREATEDIALOG_H
#include "ui/QTabularCreateDialogBase.h"

class QTabularCreate;

class QTabularCreateDialog : public QTabularCreateDialogBase
{ Q_OBJECT

public:
	QTabularCreateDialog(QTabularCreate * form, QWidget * parent = 0, const char * name = 0, bool modal = FALSE, WFlags fl = 0);
	~QTabularCreateDialog();

protected slots:
	void insert_tabular();
	void cancel_adaptor();
	virtual void colsChanged(int);
	virtual void rowsChanged(int);
   
private:
	QTabularCreate * form_;
};

#endif // QTABULARCREATEDIALOG_H
