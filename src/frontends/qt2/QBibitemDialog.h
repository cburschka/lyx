/**
 * \file QBibitemDialog.h
 * Copyright 2001 the LyX Team
 * Read the file COPYING
 *
 * \author John Levon <moz@compsoc.man.ac.uk>
 */

#ifndef QBIBITEMDIALOG_H
#define QBIBITEMDIALOG_H

#include "ui/QBibitemDialogBase.h"

class QBibitem;

class QBibitemDialog : public QBibitemDialogBase
{ Q_OBJECT

public:
	QBibitemDialog(QBibitem * form);

protected slots:
	virtual void change_adaptor();


protected:
	virtual void closeEvent(QCloseEvent * e);

private:
	QBibitem * form_;
};

#endif // QBIBITEMDIALOG_H
