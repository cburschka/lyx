/**
 * \file pspell.C
 * Copyright 2001 the LyX Team
 * Read the file COPYING
 *
 * \author Kevin Atkinson
 * \author John Levon <levon@movementarian.org>
 */

#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#ifdef USE_PSPELL

#include "support/LAssert.h"

#define USE_ORIGINAL_MANAGER_FUNCS 1
#include <pspell/pspell.h>

#include "pspell.h"


PSpell::PSpell(BufferParams const & params, string const & lang)
	: sc(0), els(0), spell_error_object(0), alive_(false)
{
	PspellConfig * config = new_pspell_config();
	config->replace("language-tag", lang.c_str());
	spell_error_object = new_pspell_manager(config);
	if (pspell_error_number(spell_error_object) == 0) {
		sc = to_pspell_manager(spell_error_object);
		spell_error_object = 0;
		alive_ = true;
	}
}


PSpell::~PSpell()
{
	cleanUp();
	close();
	if (els)
		delete_pspell_string_emulation(els);
}


void PSpell::cleanUp()
{
	if (spell_error_object) {
		delete_pspell_can_have_error(spell_error_object);
		spell_error_object = 0;
	}
}


enum PSpell::Result PSpell::check(string const & word)
{
	Result res = UNKNOWN;
 
	if (!sc)
		return res;

	int word_ok = pspell_manager_check(sc, word.c_str());
	lyx::Assert(word_ok != -1);

	if (word_ok) {
		res = OK;
	} else {
		PspellWordList const * sugs =
			pspell_manager_suggest(sc, word.c_str());
		lyx::Assert(sugs != 0);
		els = pspell_word_list_elements(sugs);
		if (pspell_word_list_empty(sugs))
			res = UNKNOWN;
		else
			res = MISSED;
	}
	return res;
}


void PSpell::close()
{
	if (sc)
		pspell_manager_save_all_word_lists(sc);
}


void PSpell::insert(string const & word)
{
	if (sc)
		pspell_manager_add_to_personal(sc, word.c_str());
}


void PSpell::accept(string const & word)
{
	if (sc)
		pspell_manager_add_to_session(sc, word.c_str());
}


string const PSpell::nextMiss()
{
	char const * str = 0;
 
	if (els)
		str = pspell_string_emulation_next(els);
	if (str)
		return str;
	return "";
}


string const PSpell::error()
{
	char const * err = 0;
 
	if (pspell_error_number(spell_error_object) != 0) {
		err = pspell_error_message(spell_error_object);
	}

	if (err)
		return err;
	return "";
}

#endif // USE_PSPELL
