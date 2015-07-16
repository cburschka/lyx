// -*- C++ -*-
/**
 * \file WordLangTuple.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef WORD_LANG_TUPLE_H
#define WORD_LANG_TUPLE_H

#include "Language.h"

#include "support/docstring.h"


namespace lyx {


/**
 * A word and its given language.
 * This is used for spellchecking
 * and thesaurus.
 */
class WordLangTuple {
public:
	WordLangTuple() : lang_(0) {}

	WordLangTuple(docstring const & w, Language const * l)
		: word_(w), lang_(l)
	{}

	/// return the word
	docstring const & word() const {
		return word_;
	}

	/// return its language code
	Language const * lang() const {
		return lang_;
	}

private:
	/// the word
	docstring word_;
	/// language of word
	Language const * lang_;
};


} // namespace lyx

#endif // WORD_LANG_TUPLE_H
