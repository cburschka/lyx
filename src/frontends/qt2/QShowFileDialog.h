/**
 * \file QShowFileDialog.h
 * Copyright 2001 the LyX Team
 * Read the file COPYING
 *
 * \author John Levon <moz@compsoc.man.ac.uk>
 */

#ifndef QSHOWFILEDIALOG_H
#define QSHOWFILEDIALOG_H

#include <config.h>

#include "ui/QShowFileDialogBase.h"

class QShowFile;

class QShowFileDialog : public QShowFileDialogBase
{ Q_OBJECT

public:
	QShowFileDialog(QShowFile * form);

protected:
	virtual void closeEvent(QCloseEvent * e);

private:
	QShowFile * form_;
};

#endif // QSHOWFILEDIALOG_H
