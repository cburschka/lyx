// -*- C++ -*-
/**
 * \file GuiSearch.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 * \author Angus Leeming
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef GUISEARCH_H
#define GUISEARCH_H

#include "GuiDialog.h"
#include "ui_SearchUi.h"

namespace lyx {
namespace frontend {

class GuiSearch : public GuiDialog, public Ui::SearchUi
{
	Q_OBJECT

public:
	GuiSearch(GuiView & lv);

private Q_SLOTS:
	void findChanged();
	void findClicked();
	void replaceClicked();
	void replaceallClicked();

private:
	void showEvent(QShowEvent * e);
	///
	bool initialiseParams(std::string const &) { return true; }
	void clearParams() {}
	void dispatchParams() {}
	bool isBufferDependent() const { return true; }

	/// Searches occurence of string
	void find(docstring const & search,
		  bool casesensitive, bool matchword, bool forward);

	/// Replaces occurence of string
	void replace(docstring const & search, docstring const & replace,
		     bool casesensitive, bool matchword,
		     bool forward, bool all);
};

} // namespace frontend
} // namespace lyx

#endif // GUISEARCH_H
