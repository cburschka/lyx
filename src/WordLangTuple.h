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

/**
 * A word and its given language code ("en_US").
 * This is used for spellchecking.
 */
class WordLangTuple {
public:
	WordLangTuple() {}

	WordLangTuple(std::string const & w, std::string const & c)
		: word_(w), code_(c)
	{}

	/// return the word
	std::string const & word() const {
		return word_;
	}

	/// return its language code
	std::string const & lang_code() const {
		return code_;
	}

private:
	/// the word
	std::string word_;
	/// language code of word
	std::string code_;
};

#endif // WORD_LANG_TUPLE_H
