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
	QIndexDialog(QIndex * form);
	~QIndexDialog();

public slots:
	virtual void ok_adaptor();
	virtual void close_adaptor();
	virtual void change_adaptor();

protected:
	virtual void closeEvent(QCloseEvent * e);

private:
	QIndex * form_;
};

#endif // QINDEXDIALOG_H
