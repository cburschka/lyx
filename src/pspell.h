// -*- C++ -*-
/**
 * \file pspell.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Kevin Atkinson
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef LYX_PSPELL_H
#define LYX_PSPELL_H

#include "SpellBase.h"

#include <map>

class PspellManager;
class PspellStringEmulation;
class PspellCanHaveError;
class PspellConfig;

class BufferParams;


class PSpell : public SpellBase {
public:
	/**
	 * Initialise the spellchecker with the given buffer params and language.
	 */
	PSpell(BufferParams const & params, std::string const & lang);

	virtual ~PSpell();

	/**
	 * return true if the spellchecker instance still exists
	 * Always true for pspell, since there is no separate process
	 */
	virtual bool alive() { return true; }

	/// check the given word and return the result
	virtual enum Result check(WordLangTuple const &);

	/// insert the given word into the personal dictionary
	virtual void insert(WordLangTuple const &);

	/// accept the given word temporarily
	virtual void accept(WordLangTuple const &);

	/// return the next near miss after a MISSED result
	virtual std::string const nextMiss();

	/// give an error message on messy exit
	virtual std::string const error();

private:
	/// add a manager of the given language
	void addManager(std::string const & lang);

	struct Manager {
		PspellManager * manager;
		PspellConfig * config;
	};

	typedef std::map<std::string, struct Manager> Managers;

	/// the managers
	Managers managers_;

	/// FIXME
	PspellStringEmulation * els;
	/// FIXME
	PspellCanHaveError * spell_error_object;
};

#endif // PSPELL_H
