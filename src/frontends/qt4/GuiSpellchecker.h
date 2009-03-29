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

class SpellChecker;

namespace frontend {

class GuiSpellchecker : public GuiDialog, public Ui::SpellcheckerUi
{
	Q_OBJECT

public:
	GuiSpellchecker(GuiView & lv);
	~GuiSpellchecker();

public Q_SLOTS:
	void suggestionChanged(QListWidgetItem *);

private Q_SLOTS:
	void accept();
	void add();
	void ignore();
	void replace();
	void replaceChanged(const QString &);
	void reject();

private:
	/// update from controller
	void partialUpdate(int id);
	///
	void updateContents();

	///
	enum State {
		SPELL_PROGRESSED, //< update progress bar
		SPELL_FOUND_WORD //< found a bad word
	};

	///
	bool initialiseParams(std::string const & data);
	///
	void clearParams();
	/// Not needed here
	void dispatchParams() {}
	///
	bool isBufferDependent() const { return true; }
	///
	bool exitEarly() const { return exitEarly_; }

	/// replace word with replacement
	void replace(docstring const &);

	/// replace all occurances of word
	void replaceAll(docstring const &);
	/// insert word in personal dictionary
	void insert();
	/// ignore all occurances of word
	void ignoreAll();
	/// check text until next misspelled/unknown word
	/// returns true when finished
	void check();
	/// get suggestion
	docstring getSuggestion() const;
	/// get word
	docstring getWord() const;
	/// returns progress value
	int getProgress() const { return oldval_; }
	/// returns word count
	int getCount() const { return count_; }
	/// give error message is spellchecker dies
	bool checkAlive();
	/// show count of checked words at normal exit
	void showSummary();

	/// set to true when spellchecking is finished
	bool exitEarly_;
	/// current word being checked and lang code
	WordLangTuple word_;
	/// values for progress
	int oldval_;
	int newvalue_;
	/// word count
	int count_;
	/// The actual spellchecker object
	SpellChecker * speller_;
};

} // namespace frontend
} // namespace lyx

#endif // GUISPELLCHECKER_H
