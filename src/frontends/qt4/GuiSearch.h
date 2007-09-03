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

#ifndef GUISEARCH_H
#define GUISEARCH_H

#include "GuiDialogView.h"
#include "ControlSearch.h"
#include "ui_SearchUi.h"

#include <string>

#include <QDialog>

namespace lyx {
namespace frontend {

class GuiSearch;

class GuiSearchDialog : public QDialog, public Ui::SearchUi {
	Q_OBJECT
public:
	GuiSearchDialog(GuiSearch * form);

	virtual void showView();
protected Q_SLOTS:
	void findChanged();
	void findClicked();
	void replaceClicked();
	void replaceallClicked();
protected:
	virtual void closeEvent(QCloseEvent * e);

private:
	GuiSearch * form_;
};


class GuiSearch : public GuiView<GuiSearchDialog>
{
public:
	///
	friend class GuiSearchDialog;
	///
	GuiSearch(GuiDialog &);
	/// parent controller
	ControlSearch & controller()
	{ return static_cast<ControlSearch &>(this->getController()); }
	/// parent controller
	ControlSearch const & controller() const
	{ return static_cast<ControlSearch const &>(this->getController()); }
private:
	/// Apply changes
	virtual void applyView() {}
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

#endif // GUISEARCH_H
