// -*- C++ -*-
/**
 * \file ControlSpellchecker.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Edwin Leuven
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef CONTROLSPELLCHECKER_H
#define CONTROLSPELLCHECKER_H

#include <boost/scoped_ptr.hpp>

#include "ControlDialog_impl.h"
#include "LString.h"
#include "WordLangTuple.h"

class SpellBase;

/** A controller for Spellchecker dialogs.
 */
class ControlSpellchecker : public ControlDialogBD {
public:
	enum State {
		SPELL_PROGRESSED, //< update progress bar
		SPELL_FOUND_WORD //< found a bad word
	};

	ControlSpellchecker(LyXView &, Dialogs &);

	~ControlSpellchecker();

	/// replace word with replacement
	void replace(string const &);

	/// replace all occurances of word
	void replaceAll(string const &);

	/// insert word in personal dictionary
	void insert();

	/// ignore all occurances of word
	void ignoreAll();

	/// check text until next misspelled/unknown word
	/// returns true when finished
	void check();

	/// get suggestion
	string const getSuggestion() const;

	/// get word
	string const getWord() const;

	/// returns progress value
	int getProgress() const { return oldval_; }

	/// returns word count
	int getCount()  const { return count_; }

private:
	/// give error message is spellchecker dies
	bool checkAlive();

	/// start a spell-checking session
	void startSession();

	/// end a spell-checking session
	void endSession();

	/// show count of checked words at normal exit
	void showSummary();

	/// set the params before show or update
	void setParams();
	/// clean-up on hide.
	void clearParams();

	/// not needed.
	virtual void apply() {}

	/// current word being checked and lang code
	WordLangTuple word_;

	/// values for progress
	float newval_;
	int oldval_;
	int newvalue_;

	/// word count
	int count_;

	/// The actual spellchecker object
	boost::scoped_ptr<SpellBase> speller_;
};

#endif // CONTROLSPELLCHECKER_H
