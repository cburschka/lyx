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
	void findClicked(bool const backwards = false);
	void replaceClicked();
	void replaceallClicked();

private:
	///
	void keyPressEvent(QKeyEvent * e) override;
	///
	void showEvent(QShowEvent * e) override;
	///
	bool initialiseParams(std::string const &) override { return true; }
	void clearParams() override {}
	void dispatchParams() override {}
	bool isBufferDependent() const override { return true; }

	/// Searches occurrence of string
	void find(docstring const & search,
		  bool casesensitive, bool matchword, bool forward);

	/// Replaces occurrence of string
	void replace(docstring const & search, docstring const & replace,
		     bool casesensitive, bool matchword,
		     bool forward, bool all);
};

} // namespace frontend
} // namespace lyx

#endif // GUISEARCH_H
