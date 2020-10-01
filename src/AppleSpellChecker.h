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
	enum Result check(WordLangTuple const &) override;
	void suggest(WordLangTuple const &, docstring_list &) override;
	void stem(WordLangTuple const &, docstring_list &) override {}
	void insert(WordLangTuple const &) override;
	void remove(WordLangTuple const &) override;
	void accept(WordLangTuple const &) override;
	bool hasDictionary(Language const * lang) const override;
	int numDictionaries() const override;
	bool canCheckParagraph() const override { return true; }
	int numMisspelledWords() const override;
	void misspelledWord(int index, int & start, int & length) const override;
	docstring const error() override;
	void advanceChangeNumber() override;
	//@}

private:
	struct Private;
	Private * d;
};


} // namespace lyx

#endif // LYX_APPLESPELL_H
