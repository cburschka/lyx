/**
 * \file QIndexDialog.h
 * Copyright 2001 the LyX Team
 * Read the file COPYING
 *
 * \author John Levon <moz@compsoc.man.ac.uk>
 */

#ifndef QINDEXDIALOG_H
#define QINDEXDIALOG_H
 
#include <config.h>
 
#include "ui/QIndexDialogBase.h"

class QIndex;

class QIndexDialog : public QIndexDialogBase
{ Q_OBJECT

public:
	QIndexDialog(QIndex * form, QWidget * parent = 0, const char * name = 0, bool modal = FALSE, WFlags fl = 0);
	~QIndexDialog();

public slots:
	void apply_adaptor();
	void close_adaptor();

protected:
	void closeEvent(QCloseEvent * e);

private:
	QIndex * form_;
};

#endif // QINDEXDIALOG_H
