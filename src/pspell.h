/**
 * \file pspell.h
 * Copyright 2001 the LyX Team
 * Read the file COPYING
 *
 * \author Kevin Atkinson
 * \author John Levon <levon@movementarian.org>
 */

#ifndef LYX_PSPELL_H
#define LYX_PSPELL_H

#include <map>
 
#include "SpellBase.h"

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
	PSpell(BufferParams const & params, string const & lang);

	virtual ~PSpell();

	/**
	 * return true if the spellchecker instance still exists
	 * Always true for pspell, since there is no separate process
	 */
	virtual bool alive() { return true; }

	/// clean up on messy exit
	virtual void cleanUp();

	/// check the given word and return the result
	virtual enum Result check(WordLangTuple const &);

	/// finish this spellchecker instance
	virtual void close();

	/// insert the given word into the personal dictionary
	virtual void insert(WordLangTuple const &);

	/// accept the given word temporarily
	virtual void accept(WordLangTuple const &);

	/// return the next near miss after a MISSED result
	virtual string const nextMiss();

	/// give an error message on messy exit 
	virtual string const error();

private:
	/// add a manager of the given language
	void addManager(string const & lang);

	struct Manager {
		PspellManager * manager;
		PspellConfig * config;
	};
 
	typedef std::map<string, struct Manager> Managers;

	/// the managers
	Managers managers_;
 
	/// FIXME
	PspellStringEmulation * els;
	/// FIXME
	PspellCanHaveError * spell_error_object;
};

#endif // PSPELL_H
