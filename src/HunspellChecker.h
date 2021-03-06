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
	enum Result check(WordLangTuple const &,
			  std::vector<WordLangTuple> const &) override;
	void suggest(WordLangTuple const &, docstring_list &) override;
	void stem(WordLangTuple const &, docstring_list &) override;
	void insert(WordLangTuple const &) override;
	void remove(WordLangTuple const &) override;
	void accept(WordLangTuple const &) override;
	bool hasDictionary(Language const * lang) const override;
	int numDictionaries() const override;
	docstring const error() override;
	void advanceChangeNumber() override;
	///@}

private:
	struct Private;
	Private * d;
};


} // namespace lyx

#endif // LYX_Hunspell_H
