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

#include "support/std_string.h"

/**
 * A word and its given language code ("en_US").
 * This is used for spellchecking.
 */
class WordLangTuple {
public:
	WordLangTuple() {}

	WordLangTuple(string const & w, string const & c)
		: word_(w), code_(c)
	{}

	/// return the word
	string const & word() const {
		return word_;
	}

	/// return its language code
	string const & lang_code() const {
		return code_;
	}

private:
	/// the word
	string word_;
	/// language code of word
	string code_;
};

#endif // WORD_LANG_TUPLE_H
