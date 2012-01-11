// -*- C++ -*-
/**
 * \file AppleSpellChecker.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Stephan Witt
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef LYX_APPLESPELL_H
#define LYX_APPLESPELL_H

#include "SpellChecker.h"

namespace lyx {

class AppleSpellChecker : public SpellChecker
{
public:
	AppleSpellChecker();
	~AppleSpellChecker();

	/// \name SpellChecker inherited methods
	//@{
	enum Result check(WordLangTuple const &);
	void suggest(WordLangTuple const &, docstring_list &);
	void insert(WordLangTuple const &);
	void remove(WordLangTuple const &);
	void accept(WordLangTuple const &);
	bool hasDictionary(Language const * lang) const;
	int numDictionaries() const;
	bool canCheckParagraph() const { return true; }
	int numMisspelledWords() const;
	void misspelledWord(int index, int & start, int & length) const;
	docstring const error();
	void advanceChangeNumber();
	//@}

private:
	struct Private;
	Private * d;
};


} // namespace lyx

#endif // LYX_APPLESPELL_H
