// -*- C++ -*-
/**
 * \file QIncludeDialog.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS
 */

#ifndef QINCLUDEDIALOG_H
#define QINCLUDEDIALOG_H

#ifdef __GNUG__
#pragma interface
#endif

#include "ui/QIncludeDialogBase.h"

class QInclude;

class QIncludeDialog : public QIncludeDialogBase {
	Q_OBJECT
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
