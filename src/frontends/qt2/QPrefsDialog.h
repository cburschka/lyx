// -*- C++ -*-
/**
 * \file QPrefsDialog.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS
 */

#ifndef QPREFSDIALOG_H
#define QPREFSDIALOG_H

#ifdef __GNUG__
#pragma interface
#endif

#include "ui/QPrefsDialogBase.h"

class QPrefs;

class QPrefsDialog : public QPrefsDialogBase {
	Q_OBJECT
public:
	friend class QPrefs;

	QPrefsDialog(QPrefs *);
 
	~QPrefsDialog();

protected:
	void closeEvent(QCloseEvent * e);
 
private:
	QPrefs * form_;
};

#endif // PREFSDIALOG_H
