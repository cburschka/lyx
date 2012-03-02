// -*- C++ -*-
/**
 * \file EnchantChecker.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Caolán McNamara
 * \author Jürgen Spitzmüller
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef LYX_ENCHANT_H
#define LYX_ENCHANT_H

#include "SpellChecker.h"

namespace enchant {
    class Dict;
}

namespace lyx {

class BufferParams;


class EnchantChecker : public SpellChecker {
public:
	EnchantChecker();
	~EnchantChecker();

	/// SpellChecker inherited methods.
	///@{
	enum Result check(WordLangTuple const &);
	void suggest(WordLangTuple const &, docstring_list &);
	void stem(WordLangTuple const &, docstring_list &) {};
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

#endif // LYX_ENCHANT_H
