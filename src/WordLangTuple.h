/**
 * \file WordLangTuple.h
 * Copyright 2002 the LyX Team
 * Read the file COPYING
 *
 * \author John Levon <levon@movementarian.org>
 */

#ifndef WORD_LANG_TUPLE_H
#define WORD_LANG_TUPLE_H
 
#include <config.h>
#include "LString.h" 
 
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
	string const word() const {
		return word_;
	}

	/// return its language code
	string const lang_code() const {
		return code_;
	}

private:
	/// the word
	string word_;
	/// language code of word
	string code_;
};
 
#endif // WORD_LANG_TUPLE_H
