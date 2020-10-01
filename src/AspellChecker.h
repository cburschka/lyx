// -*- C++ -*-
/**
 * \file AspellChecker.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Kevin Atkinson
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef LYX_ASPELL_H
#define LYX_ASPELL_H

#include "SpellChecker.h"

namespace lyx {

class AspellChecker : public SpellChecker
{
public:
	AspellChecker();
	~AspellChecker();

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
	docstring const error() override;
	void advanceChangeNumber() override;
	//@}

private:
	struct Private;
	Private * d;
};


} // namespace lyx

#endif // LYX_ASPELL_H
