// -*- C++ -*-
/**
 * \file QSearch.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef QSEARCH_H
#define QSEARCH_H

#include "QDialogView.h"
#include "ui/SearchUi.h"
#include <string>

#include <QDialog>

namespace lyx {
namespace frontend {

class ControlSearch;

class QSearch;

class QSearchDialog : public QDialog, public Ui::QSearchUi {
	Q_OBJECT
public:
	QSearchDialog(QSearch * form);

protected Q_SLOTS:
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


class QSearch
	: public QController<ControlSearch, QView<QSearchDialog> >
{
public:
	///
	friend class QSearchDialog;
	///
	QSearch(Dialog &);
private:
	/// Apply changes
	virtual void apply() {}
	/// update
	virtual void update_contents();
	/// build the dialog
	virtual void build_dialog();

	void find(docstring const & str, bool casesens,
		  bool words, bool backwards);

	void replace(docstring const & findstr,
		     docstring const & replacestr,
		     bool casesens, bool words, bool backwards, bool all);
};

} // namespace frontend
} // namespace lyx

#endif // QSEARCH_H
