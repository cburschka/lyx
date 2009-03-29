// -*- C++ -*-
/**
 * \file ASpell_local.h
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

#include <map>
#include <string>

struct AspellSpeller;
struct AspellStringEnumeration;
struct AspellCanHaveError;
struct AspellConfig;

namespace lyx {


class ASpell : public SpellChecker
{
public:
	ASpell();
	~ASpell();

	/// check the given word and return the result
	enum Result check(WordLangTuple const &);

	/// insert the given word into the personal dictionary
	void insert(WordLangTuple const &);

	/// accept the given word temporarily
	void accept(WordLangTuple const &);

	/// return the next near miss after a SUGGESTED_WORDS result
	docstring const nextMiss();

	/// give an error message on messy exit
	docstring const error();

private:
	/// add a speller of the given language
	void addSpeller(std::string const & lang);

	struct Speller {
		AspellSpeller * speller;
		AspellConfig * config;
	};

	typedef std::map<std::string, Speller> Spellers;

	/// the spellers
	Spellers spellers_;

	/// FIXME
	AspellStringEnumeration * els;
	/// FIXME
	AspellCanHaveError * spell_error_object;
};


} // namespace lyx

#endif // LYX_ASPELL_H
