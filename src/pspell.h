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

#include "SpellBase.h"

class PspellManager;
class PspellStringEmulation;
class PspellCanHaveError;

class BufferParams;


class PSpell : public SpellBase {
public:
	/**
	 * Initialise the spellchecker with the given buffer params and language.
	 */
	PSpell(BufferParams const & params, string const & lang);

	virtual ~PSpell();

	/// return true if the spellchecker instance still exists
	virtual bool alive() { return alive_; }

	/// clean up on messy exit
	virtual void cleanUp();

	/// check the given word and return the result
	virtual enum Result check(string const & word);

	/// finish this spellchecker instance
	virtual void close();

	/// insert the given word into the personal dictionary
	virtual void insert(string const & word);

	/// accept the given word temporarily
	virtual void accept(string const & word);

	/// return the next near miss after a MISSED result
	virtual string const nextMiss();

	/// give an error message on messy exit 
	virtual string const error();

private:
	/// main manager
	PspellManager * sc;
	/// FIXME
	PspellStringEmulation * els;
	/// FIXME
	PspellCanHaveError * spell_error_object;
	/// initialised properly ?
	bool alive_;
};

#endif // PSPELL_H
