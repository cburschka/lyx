// -*- C++ -*-
/**
 * \file QSearchDialog.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Edwin Leuven
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef QSEARCHDIALOG_H
#define QSEARCHDIALOG_H

#include "ui/QSearchDialogBase.h"
#include <string>

class QCloseEvent;
class QComboBox;

namespace lyx {
namespace frontend {

class QSearch;

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
	void remember(std::string const & find, QComboBox & combo);

	QSearch * form_;
};

} // namespace frontend
} // namespace lyx

#endif // QSEARCHDIALOG_H
