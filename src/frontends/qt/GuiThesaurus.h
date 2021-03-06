// -*- C++ -*-
/**
 * \file GuiThesaurus.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef GUITHESAURUS_H
#define GUITHESAURUS_H

#include "GuiDialog.h"
#include "Thesaurus.h"
#include "ui_ThesaurusUi.h"

class QTreeWidgetItem;

namespace lyx {
namespace frontend {

class GuiThesaurus : public GuiDialog, public Ui::ThesaurusUi
{
	Q_OBJECT

public:
	GuiThesaurus(GuiView & lv);

	void checkStatus() override;

private Q_SLOTS:
	void change_adaptor();
	void entryChanged();
	void replaceClicked();
	void selectionChanged();
	void selectionClicked(QTreeWidgetItem *, int);
	void itemClicked(QTreeWidgetItem *, int);

private:
	/// update
	void updateContents() override;
	///
	void updateLists();
	///
	bool initialiseParams(std::string const & data) override;
	///
	void clearParams() override;
	///
	void dispatchParams() override {}
	///
	bool isBufferDependent() const override { return true; }

	/// replace the particular string
	void replace(docstring const & newstr);

	/// get meanings
	Thesaurus::Meanings const & getMeanings(WordLangTuple const & wl);

private:
	/// last string looked up
	docstring laststr_;

	/// entries for last string
	Thesaurus::Meanings meanings_;

	/// original string
	docstring text_;

	/// language
	docstring lang_;

	/// not needed.
	void apply() override {}
};

} // namespace frontend
} // namespace lyx

#endif // GUITHESAURUS_H
