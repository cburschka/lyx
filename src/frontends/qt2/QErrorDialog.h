/**
 * \file QErrorDialog.h
 * Copyright 2001 the LyX Team
 * Read the file COPYING
 *
 * \author John Levon <moz@compsoc.man.ac.uk>
 */

#ifndef QERRORDIALOG_H
#define QERRORDIALOG_H

#include <config.h>

#include "ui/QErrorDialogBase.h"

class QError;

class QErrorDialog : public QErrorDialogBase
{ Q_OBJECT

public:
	QErrorDialog(QError * form);

protected:
	virtual void closeEvent(QCloseEvent * e);

private:
	QError * form_;
};

#endif // QERRORDIALOG_H
