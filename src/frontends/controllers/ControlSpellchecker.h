// -*- C++ -*-
/**
 * \file ControlSpellchecker.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Edwin Leuven
 *
 * Full author contact details are available in file CREDITS
 */

#ifndef CONTROLSPELLCHECKER_H
#define CONTROLSPELLCHECKER_H

#ifdef __GNUG__
#pragma interface
#endif

#include "ControlDialog_impl.h"
#include "LString.h"
#include "WordLangTuple.h"

class SpellBase;

/** A controller for Spellchecker dialogs.
 */
class ControlSpellchecker : public ControlDialogBD {
public:
	///
	ControlSpellchecker(LyXView &, Dialogs &);

	/// replace word with replacement
	void replace(string const &);

	/// replace all occurances of word
	void replaceAll(string const &);

	/// insert word in personal dictionary
	void insert();

	/// ignore all occurances of word
	void ignoreAll();

	/// stop checking
	void stop();

	/// check text until next misspelled/unknown word
	void check();

	/// get suggestion
	string const getSuggestion() const;

	/// get word
	string const getWord() const;

	/// returns progress value
	int getProgress() const { return oldval_; }

	/// returns exit message
	string const getMessage()  const { return message_; }

	/// returns word count
	int getCount()  const { return count_; }

private:
	/// set the params before show or update
	void setParams();
	/// clean-up on hide.
	void clearParams();

	/// not needed.
	virtual void apply() {}

	/// right to left
	bool rtl_;

	/// current word being checked and lang code
	WordLangTuple word_;

	/// values for progress
	float newval_;
	int oldval_;
	int newvalue_;

	/// word count
	int count_;

	/// exit message
	string message_;

	/// set to true to stop checking
	bool stop_;

	/// The actual spellchecker object
	SpellBase * speller_;
};

#endif // CONTROLSPELLCHECKER_H
