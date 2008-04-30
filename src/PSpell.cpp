/**
 * \file PSpell.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Kevin Atkinson
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "support/debug.h"

#define USE_ORIGINAL_MANAGER_FUNCS 1
// new aspell pspell missing extern "C"
extern "C" {
#include <pspell/PSpell.h>
}

#include "PSpell.h"
#include "WordLangTuple.h"

#include "support/lassert.h"

using namespace std;
using namespace lyx::support;

namespace lyx {


PSpell::PSpell(BufferParams const &, string const & lang)
	: els(0), spell_error_object(0)
{
	addManager(lang);
	LYXERR(Debug::GUI, "created pspell");
}


PSpell::~PSpell()
{
	LYXERR(Debug::GUI, "killed pspell");

	if (spell_error_object) {
		delete_pspell_can_have_error(spell_error_object);
		spell_error_object = 0;
	}

	if (els)
		delete_pspell_string_emulation(els);

	Managers::iterator it = managers_.begin();
	Managers::iterator end = managers_.end();

	for (; it != end; ++it) {
		pspell_manager_save_all_word_lists(it->second.manager);
		delete_pspell_manager(it->second.manager);
		delete_pspell_config(it->second.config);
	}
}


void PSpell::addManager(string const & lang)
{
	PspellConfig * config = new_pspell_config();
	pspell_config_replace(config, "language-tag", lang.c_str());
	pspell_config_replace(config, "encoding", "utf-8");
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
	Result res = UNKNOWN_WORD;

	Managers::iterator it = managers_.find(word.lang_code());
	if (it == managers_.end()) {
		addManager(word.lang_code());
		it = managers_.find(word.lang_code());
		// FIXME
		if (it == managers_.end())
			return res;
	}

	PspellManager * m = it->second.manager;

	int word_ok = pspell_manager_check(m, to_utf8(word.word()).c_str());
	LASSERT(word_ok != -1, /**/);

	if (word_ok) {
		res = OK;
	} else {
		PspellWordList const * sugs =
			pspell_manager_suggest(m, to_utf8(word.word()).c_str());
		LASSERT(sugs != 0, /**/);
		els = pspell_word_list_elements(sugs);
		if (pspell_word_list_empty(sugs))
			res = UNKNOWN_WORD;
		else
			res = SUGGESTED_WORDS;
	}
	return res;
}


void PSpell::insert(WordLangTuple const & word)
{
	Managers::iterator it = managers_.find(word.lang_code());
	if (it != managers_.end())
		pspell_manager_add_to_personal(it->second.manager, to_utf8(word.word()).c_str());
}


void PSpell::accept(WordLangTuple const & word)
{
	Managers::iterator it = managers_.find(word.lang_code());
	if (it != managers_.end())
		pspell_manager_add_to_session(it->second.manager, to_utf8(word.word()).c_str());
}


docstring const PSpell::nextMiss()
{
	char const * str = 0;

	if (els)
		str = pspell_string_emulation_next(els);
	if (str)
		return from_utf8(str);
	return docstring();
}


docstring const PSpell::error()
{
	char const * err = 0;

	if (spell_error_object && pspell_error_number(spell_error_object) != 0) {
		err = pspell_error_message(spell_error_object);
	}

	if (err)
		return from_utf8(err);
	return docstring();
}


} // namespace lyx
