// -*- C++ -*-
/**
 * \file QSearchDialog.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Edwin Leuven
 *
 * Full author contact details are available in file CREDITS
 */

#ifndef QSEARCHDIALOG_H
#define QSEARCHDIALOG_H

#ifdef __GNUG__
#pragma interface
#endif

#include "ui/QSearchDialogBase.h"
#include "LString.h"

class QSearch;
class QCloseEvent;
class QComboBox;

class QSearchDialog : public QSearchDialogBase {
	Q_OBJECT
public:
	QSearchDialog(QSearch * form);

	virtual void show();
protected slots:
	void findChanged();
	void findClicked();
	void replaceClicked();
	void replaceallClicked();
protected:
	virtual void closeEvent(QCloseEvent * e);

private:
	// add a string to the combo if needed
	void remember(string const & find, QComboBox & combo);

	QSearch * form_;
};

#endif // QSEARCHDIALOG_H
