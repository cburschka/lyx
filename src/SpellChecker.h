// -*- C++ -*-
/**
 * \file SpellChecker.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author unknown
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef SPELL_BASE_H
#define SPELL_BASE_H

#include "support/strfwd.h"


namespace lyx {

class BufferParams;
class Language;
class WordLangTuple;
class docstring_list;

/**
 * Pure virtual base class of all spellchecker implementations.
 */
class SpellChecker {
public:

	/// the result from checking a single word
	enum Result  {
		/// word is correct
		WORD_OK = 1,
		/// root of given word was found
		ROOT_FOUND,
		/// word found through compound formation
		COMPOUND_WORD,
		/// word not found
		UNKNOWN_WORD,
		/// number of other ignored "word"
		IGNORED_WORD,
		/// number of personal dictionary "word"
		LEARNED_WORD,
		/// missing dictionary for language
		NO_DICTIONARY
	};

	SpellChecker() : change_number_(0) {}

	virtual ~SpellChecker() {}

	/// does the spell check failed
	static bool misspelled(Result res) {
		return res != WORD_OK
			&& res != IGNORED_WORD
			&& res != NO_DICTIONARY
			&& res != LEARNED_WORD; }

	/// check the given word of the given lang code and return the result
	virtual enum Result check(WordLangTuple const &) = 0;

	/// Gives suggestions.
	virtual void suggest(WordLangTuple const &, docstring_list & suggestions) = 0;

	/// Lemmatizing: return stem of word (used by Thesaurus).
	virtual void stem(WordLangTuple const &, docstring_list & suggestions) = 0;

	/// insert the given word into the personal dictionary
	virtual void insert(WordLangTuple const &) = 0;

	/// remove the given word from the personal dictionary
	virtual void remove(WordLangTuple const &) = 0;

	/// accept the given word temporarily
	virtual void accept(WordLangTuple const &) = 0;

	/// check if dictionary exists
	virtual bool hasDictionary(Language const *) const = 0;

	/// how many valid dictionaries were found
	virtual int numDictionaries() const = 0;
	
	/// if speller can spell check whole paragraph return true
	virtual bool canCheckParagraph() const { return false; }

	/// count of misspelled words
	virtual int numMisspelledWords() const { return 0; }

	/// start position and length of misspelled word at index
	virtual void misspelledWord(
		int /* index */,
		int & start, int & length) const
	{
		/// index is used here to make the compiler happy
		start = 0;
		length = 0;
	}

	/// give an error message on messy exit
	virtual docstring const error() = 0;

	/// spell checker state versioning support
	typedef unsigned long int ChangeNumber ;
	ChangeNumber changeNumber() const { return change_number_; }
	void changeNumber(ChangeNumber value) { change_number_ = value; }
	void nextChangeNumber() { ++change_number_; }
	virtual void advanceChangeNumber() = 0;

private:
	ChangeNumber change_number_;
};

/// Access to the singleton SpellChecker.
/// Implemented in LyX.cpp
SpellChecker * theSpellChecker();

/// Set the singleton SpellChecker engine.
/// Implemented in LyX.cpp
void setSpellChecker();

} // namespace lyx

#endif // SPELL_BASE_H
