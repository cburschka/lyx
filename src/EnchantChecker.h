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
	///@}

private:
	struct Private;
	Private * d;
};


} // namespace lyx

#endif // LYX_ENCHANT_H
