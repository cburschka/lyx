/**
 * \file QIncludeDialog.h
 * Copyright 2001 the LyX Team
 * Read the file COPYING
 *
 * \author John Levon <moz@compsoc.man.ac.uk>
 */

#ifndef QINCLUDEDIALOG_H
#define QINCLUDEDIALOG_H

#include <config.h>

#include "ui/QIncludeDialogBase.h"

class QInclude;

class QIncludeDialog : public QIncludeDialogBase
{ Q_OBJECT

public:
	QIncludeDialog(QInclude * form);

	void updateLists();

	virtual void show();
 
protected slots:
	virtual void change_adaptor();
	virtual void loadClicked();
	virtual void browseClicked();
	virtual void typeChanged(int v);

protected:
	virtual void closeEvent(QCloseEvent * e);

private:
	QInclude * form_;
};

#endif // QINCLUDEDIALOG_H
