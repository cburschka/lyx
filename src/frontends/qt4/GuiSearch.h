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

#include "GuiDialog.h"
#include "ControlSearch.h"
#include "ui_SearchUi.h"

namespace lyx {
namespace frontend {

class ControlSearch;

class GuiSearchDialog : public GuiDialog, public Ui::SearchUi
{
	Q_OBJECT

public:
	GuiSearchDialog(LyXView & lv);

private Q_SLOTS:
	void findChanged();
	void findClicked();
	void replaceClicked();
	void replaceallClicked();

private:
	void showView();
	void closeEvent(QCloseEvent * e);
	/// parent controller
	ControlSearch & controller() const;
	///
	void find(docstring const & str, bool casesens,
	  bool words, bool backwards);
	///
	void replace(docstring const & findstr,
	  docstring const & replacestr,
	  bool casesens, bool words, bool backwards, bool all);
};

} // namespace frontend
} // namespace lyx

#endif // GUISEARCH_H
