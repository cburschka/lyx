// -*- C++ -*-
/**
 * \file SpellBase.h
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

#include <string>

class BufferParams;
class WordLangTuple;

/**
 * Base class of all spell checker implementations.
 */
class SpellBase {
public:

	/// the result from checking a single word
	enum Result  {
		/// word is correct
		OK = 1,
		/// root of given word was found
		ROOT,
		/// word found through compound formation
		COMPOUND_WORD,
		/// word not found
		UNKNOWN_WORD,
		/// not found, with suggestions
		SUGGESTED_WORDS,
		/// number of other ignored "word"
		IGNORED_WORD
	};

	virtual ~SpellBase() {}

	/// return true if the spellchecker instance still exists
	virtual bool alive() = 0;

	/// check the given word of the given lang code and return the result
	virtual enum Result check(WordLangTuple const &) = 0;

	/// insert the given word into the personal dictionary
	virtual void insert(WordLangTuple const &) = 0;

	/// accept the given word temporarily
	virtual void accept(WordLangTuple const &) = 0;

	/// return the next near miss after a SUGGESTED_WORDS result
	virtual std::string const nextMiss() = 0;

	/// give an error message on messy exit
	virtual std::string const error() = 0;

};

#endif // SPELL_BASE_H
