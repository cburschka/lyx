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
// new aspell pspell missing extern "C" 
extern "C" { 
#include <pspell/pspell.h>
}

#include "pspell.h"

using std::endl;

PSpell::PSpell(BufferParams const &, string const & lang)
	: els(0), spell_error_object(0)
{
	addManager(lang); 
}


PSpell::~PSpell()
{
	cleanUp();
	close();
	if (els)
		delete_pspell_string_emulation(els);
	Managers::iterator it = managers_.begin();
	Managers::iterator end = managers_.end();
 
	for (; it != end; ++it) { 
		delete_pspell_manager(it->second.manager);
		delete_pspell_config(it->second.config);
	}
}


void PSpell::cleanUp()
{
	if (spell_error_object) {
		delete_pspell_can_have_error(spell_error_object);
		spell_error_object = 0;
	}
}


void PSpell::addManager(string const & lang)
{
	PspellConfig * config = new_pspell_config();
	pspell_config_replace(config, "language-tag", lang.c_str());
	PspellCanHaveError * err = new_pspell_manager(config);
	if (spell_error_object)
		delete_pspell_can_have_error(spell_error_object);
	spell_error_object = 0;
 
	if (pspell_error_number(err) == 0) {
		Manager m;
		m.manager = to_pspell_manager(err);
		m.config = config;
		managers_[lang] = m;
	} else {
		spell_error_object = err;
	}
}

 
enum PSpell::Result PSpell::check(WordLangTuple const & word)
{
	Result res = UNKNOWN;
 
	Managers::iterator it = managers_.find(word.lang_code());
	if (it == managers_.end()) {
		addManager(word.lang_code());
		it = managers_.find(word.lang_code());
		// FIXME
		if (it == managers_.end()) 
			return res;
	}

	PspellManager * m = it->second.manager;
 
	int word_ok = pspell_manager_check(m, word.word().c_str());
	lyx::Assert(word_ok != -1);

	if (word_ok) {
		res = OK;
	} else {
		PspellWordList const * sugs =
			pspell_manager_suggest(m, word.word().c_str());
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
	Managers::iterator it = managers_.begin();
	Managers::iterator end = managers_.end();
 
	for (; it != end; ++it) { 
		pspell_manager_save_all_word_lists(it->second.manager);
	}
}


void PSpell::insert(WordLangTuple const & word)
{
	Managers::iterator it = managers_.find(word.lang_code());
	if (it != managers_.end())
		pspell_manager_add_to_personal(it->second.manager, word.word().c_str());
}


void PSpell::accept(WordLangTuple const & word)
{
	Managers::iterator it = managers_.find(word.lang_code());
	if (it != managers_.end()) 
		pspell_manager_add_to_session(it->second.manager, word.word().c_str());
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
 
	if (spell_error_object && pspell_error_number(spell_error_object) != 0) {
		err = pspell_error_message(spell_error_object);
	}

	if (err)
		return err;
	return "";
}

#endif // USE_PSPELL
