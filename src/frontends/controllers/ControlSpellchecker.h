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

#include "Dialog.h"
#include "WordLangTuple.h"
#include <boost/scoped_ptr.hpp>

namespace lyx {

class SpellBase;

namespace frontend {

/** A controller for Spellchecker dialogs.
 */
class ControlSpellchecker : public Controller {
public:
	enum State {
		SPELL_PROGRESSED, //< update progress bar
		SPELL_FOUND_WORD //< found a bad word
	};

	ControlSpellchecker(Dialog &);
	~ControlSpellchecker();
	///
	virtual bool initialiseParams(std::string const & data);
	///
	virtual void clearParams();
	/// Not needed here
	virtual void dispatchParams() {}
	///
	virtual bool isBufferDependent() const { return true; }
	///
	virtual bool exitEarly() const { return exitEarly_; }

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
	docstring const getSuggestion() const;

	/// get word
	docstring const getWord() const;

	/// returns progress value
	int getProgress() const { return oldval_; }

	/// returns word count
	int getCount()  const { return count_; }

private:
	/// give error message is spellchecker dies
	bool checkAlive();

	/// show count of checked words at normal exit
	void showSummary();

private:
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
	boost::scoped_ptr<SpellBase> speller_;
};

} // namespace frontend
} // namespace lyx

#endif // CONTROLSPELLCHECKER_H
