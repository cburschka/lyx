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

#include "support/strfwd.h"


namespace lyx {

class BufferParams;
class WordLangTuple;

/**
 * Base class of all spellchecker implementations.
 * The class can be instantiated but will have no functionality.
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
	virtual bool alive();

	/// check the given word of the given lang code and return the result
	virtual enum Result check(WordLangTuple const &);

	/// insert the given word into the personal dictionary
	virtual void insert(WordLangTuple const &);

	/// accept the given word temporarily
	virtual void accept(WordLangTuple const &);

	/// return the next near miss after a SUGGESTED_WORDS result
	virtual docstring const nextMiss();

	/// give an error message on messy exit
	virtual docstring const error();
};


} // namespace lyx

#endif // SPELL_BASE_H
