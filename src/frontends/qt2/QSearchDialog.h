/**
 * \file QSearchDialog.h
 * Copyright 2001 the LyX Team
 * Read the file COPYING
 *
 * \author Edwin Leuven
 */

#ifndef QSEARCHDIALOG_H
#define QSEARCHDIALOG_H

#include <config.h>

#include "QSearch.h"
#include "ui/QSearchDialogBase.h"

class QCloseEvent;
class QComboBox;

class QSearchDialog : public QSearchDialogBase
{ Q_OBJECT

public:
	QSearchDialog(QSearch * form);

protected slots:
	void findChanged();
	void findClicked();
	void replaceClicked();
	void replaceallClicked();

protected:
	void closeEvent(QCloseEvent * e);

private:
	// add a string to the combo if needed
	void remember(string const & find, QComboBox & combo);

	QSearch * form_;

};

#endif // QSEARCHDIALOG_H
