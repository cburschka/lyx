// -*- C++ -*-
/**
 * \file Enchant.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Caolán McNamara
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef LYX_ENCHANT_H
#define LYX_ENCHANT_H

#include "SpellBase.h"

#include <map>
#include <string>

namespace enchant {
    class Dict;
}

namespace lyx {

class BufferParams;


class Enchant : public SpellBase {
public:
	/**
	 * Initialise the spellchecker with the given buffer params and language.
	 */
	Enchant(BufferParams const & params, std::string const & lang);

	virtual ~Enchant();

	/**
	 * return true if the spellchecker instance still exists
	 * Always true for enchant, since there is no separate process
	 */
	virtual bool alive() { return true; }

	/// check the given word and return the result
	virtual enum Result check(WordLangTuple const &);

	/// insert the given word into the personal dictionary
	virtual void insert(WordLangTuple const &);

	/// accept the given word temporarily
	virtual void accept(WordLangTuple const &);

	/// return the next near miss after a SUGGESTED_WORDS result
	virtual docstring const nextMiss();

	/// give an error message on messy exit
	virtual docstring const error();

private:
	/// add a speller of the given language
	void addSpeller(std::string const & lang);

	struct Speller {
		enchant::Dict* speller;
	};

	typedef std::map<std::string, Speller> Spellers;

	/// the spellers
	Spellers spellers_;

        std::vector<std::string> els;
};


} // namespace lyx

#endif // LYX_ENCHANT_H
