/**
 * \file QTocDialog.h
 * Copyright 2001 the LyX Team
 * Read the file COPYING
 *
 * \author John Levon <moz@compsoc.man.ac.uk>
 */

#ifndef QTOCDIALOG_H
#define QTOCDIALOG_H
 
#include "ui/QTocDialogBase.h"

class QToc;

class QTocDialog : public QTocDialogBase
{ Q_OBJECT

public:
	QTocDialog(QToc * form);
	~QTocDialog();

public slots:
	void activate_adaptor(int);
	void depth_adaptor(int);
	void select_adaptor(QListViewItem *);
	void update_adaptor();

protected:
	void closeEvent(QCloseEvent * e);

private:
	QToc * form_;
};

#endif // QTOCDIALOG_H
