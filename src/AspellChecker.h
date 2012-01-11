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
	enum Result check(WordLangTuple const &);
	void suggest(WordLangTuple const &, docstring_list &);
	void insert(WordLangTuple const &);
	void remove(WordLangTuple const &);
	void accept(WordLangTuple const &);
	bool hasDictionary(Language const * lang) const;
	int numDictionaries() const;
	docstring const error();
	void advanceChangeNumber();
	//@}

private:
	struct Private;
	Private * d;
};


} // namespace lyx

#endif // LYX_ASPELL_H
