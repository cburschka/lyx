/**
 * \file QExternalDialog.h
 * Copyright 2001 the LyX Team
 * Read the file COPYING
 *
 * \author John Levon <moz@compsoc.man.ac.uk>
 */

#ifndef QEXTERNALDIALOG_H
#define QEXTERNALDIALOG_H
 
#include <config.h>
 
#include "ui/QExternalDialogBase.h"

class QExternal;

class QExternalDialog : public QExternalDialogBase
{ Q_OBJECT

public:
	QExternalDialog(QExternal * form);

protected slots:
	virtual void change_adaptor();
	virtual void editClicked();
	virtual void viewClicked();
	virtual void updateClicked();
	virtual void browseClicked();
	virtual void templateChanged();
 
protected:
	virtual void closeEvent(QCloseEvent * e);

private:
	QExternal * form_;
};

#endif // QEXTERNALDIALOG_H
