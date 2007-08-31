// -*- C++ -*-
/**
 * \file GuiSearch.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef QSEARCH_H
#define QSEARCH_H

#include "GuiDialogView.h"
#include "ui_SearchUi.h"
#include <string>

#include <QDialog>

namespace lyx {
namespace frontend {

class ControlSearch;

class GuiSearch;

class GuiSearchDialog : public QDialog, public Ui::SearchUi {
	Q_OBJECT
public:
	GuiSearchDialog(GuiSearch * form);

	virtual void show();
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

	GuiSearch * form_;
};


class GuiSearch
	: public QController<ControlSearch, GuiView<GuiSearchDialog> >
{
public:
	///
	friend class GuiSearchDialog;
	///
	GuiSearch(Dialog &);
private:
	/// Apply changes
	virtual void apply() {}
	/// update
	virtual void update_contents() {}
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
