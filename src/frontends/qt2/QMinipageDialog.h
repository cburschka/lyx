/**
 * \file QMinipageDialog.h
 * Copyright 2001 the LyX Team
 * Read the file COPYING
 *
 * \author John Levon <moz@compsoc.man.ac.uk>
 */

#ifndef QMINIPAGEDIALOG_H
#define QMINIPAGEDIALOG_H

#include <config.h>

#include "ui/QMinipageDialogBase.h"

class QMinipage;

class QMinipageDialog : public QMinipageDialogBase
{ Q_OBJECT

public:
	QMinipageDialog(QMinipage * form);

protected slots:
	virtual void change_adaptor();

protected:
	virtual void closeEvent(QCloseEvent * e);

private:
	QMinipage * form_;
};

#endif // QMINIPAGEDIALOG_H
