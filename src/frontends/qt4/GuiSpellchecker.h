// -*- C++ -*-
/**
 * \file GuiSpellchecker.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 * \author Kalle Dalheimer
 * \author Edwin Leuven
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef GUISPELLCHECKER_H
#define GUISPELLCHECKER_H

#include "GuiDialog.h"
#include "ui_SpellcheckerUi.h"
#include "Dialog.h"
#include "WordLangTuple.h"

class QListWidgetItem;

namespace lyx {

class docstring_list;
class SpellChecker;

namespace frontend {

class GuiSpellchecker : public GuiDialog, public Ui::SpellcheckerUi
{
	Q_OBJECT

public:
	GuiSpellchecker(GuiView & lv);

public Q_SLOTS:
	void suggestionChanged(QListWidgetItem *);

private Q_SLOTS:
	/// ignore all occurances of word
	void accept();
	void add();
	void ignore();
	void replace();
	void replaceChanged(const QString &);
	void reject();

private:
	/// update from controller
	void updateSuggestions(docstring_list & words);
	///
	void updateContents();

	///
	bool initialiseParams(std::string const & data);
	///
	void clearParams() {}
	/// Not needed here
	void dispatchParams() {}
	///
	bool isBufferDependent() const { return true; }

	/// replace word with replacement
	void replace(docstring const &);

	/// replace all occurances of word
	void replaceAll(docstring const &);
	/// insert word in personal dictionary
	void insert();
	/// check text until next misspelled/unknown word
	/// returns true when finished
	void check();
	/// show count of checked words at normal exit
	void showSummary();

	/// current word being checked and lang code
	WordLangTuple word_;
	/// values for progress
	int total_;
	int progress_;
	/// word count
	int count_;
};

} // namespace frontend
} // namespace lyx

#endif // GUISPELLCHECKER_H
