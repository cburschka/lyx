// -*- C++ -*-
/**
 * \file HunspellChecker.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Abdelrazak Younes
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef LYX_HUNSPELL_H
#define LYX_HUNSPELL_H

#include "SpellChecker.h"

namespace lyx {


class HunspellChecker : public SpellChecker
{
public:
	HunspellChecker();
	~HunspellChecker();

	/// \name SpellChecker inherited methods.
	///@{
	enum Result check(WordLangTuple const &);
	void suggest(WordLangTuple const &, docstring_list &);
	void insert(WordLangTuple const &);
	void remove(WordLangTuple const &);
	void accept(WordLangTuple const &);
	bool hasDictionary(Language const * lang) const;
	int numDictionaries() const;
	docstring const error();
	void advanceChangeNumber();
	///@}

private:
	struct Private;
	Private * d;
};


} // namespace lyx

#endif // LYX_Hunspell_H
