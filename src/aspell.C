/**
 * \file aspell_local.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Kevin Atkinson
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#ifdef USE_ASPELL

#include "debug.h"

#include <aspell.h>

#include "aspell_local.h"
#include "WordLangTuple.h"


ASpell::ASpell(BufferParams const &, string const & lang)
	: els(0), spell_error_object(0)
{
	addSpeller(lang);
}


ASpell::~ASpell()
{
	if (spell_error_object) {
		delete_aspell_can_have_error(spell_error_object);
		spell_error_object = 0;
	}

	if (els)
		delete_aspell_string_enumeration(els);

	Spellers::iterator it = spellers_.begin();
	Spellers::iterator end = spellers_.end();

	for (; it != end; ++it) {
		aspell_speller_save_all_word_lists(it->second.speller);
		delete_aspell_speller(it->second.speller);
		delete_aspell_config(it->second.config);
	}
}


void ASpell::addSpeller(string const & lang)
{
	AspellConfig * config = new_aspell_config();
	aspell_config_replace(config, "language-tag", lang.c_str());
	AspellCanHaveError * err = new_aspell_speller(config);
	if (spell_error_object)
		delete_aspell_can_have_error(spell_error_object);
	spell_error_object = 0;

	if (aspell_error_number(err) == 0) {
		Speller m;
		m.speller = to_aspell_speller(err);
		m.config = config;
		spellers_[lang] = m;
	} else {
		spell_error_object = err;
	}
}


enum ASpell::Result ASpell::check(WordLangTuple const & word)
{
	Result res = UNKNOWN;

	Spellers::iterator it = spellers_.find(word.lang_code());
	if (it == spellers_.end()) {
		addSpeller(word.lang_code());
		it = spellers_.find(word.lang_code());
		// FIXME
		if (it == spellers_.end())
			return res;
	}

	AspellSpeller * m = it->second.speller;

	int word_ok = aspell_speller_check(m, word.word().c_str(), -1);
	BOOST_ASSERT(word_ok != -1);

	if (word_ok) {
		res = OK;
	} else {
		AspellWordList const * sugs =
			aspell_speller_suggest(m, word.word().c_str(), -1);
		BOOST_ASSERT(sugs != 0);
		els = aspell_word_list_elements(sugs);
		if (aspell_word_list_empty(sugs))
			res = UNKNOWN;
		else
			res = MISSED;
	}
	return res;
}


void ASpell::insert(WordLangTuple const & word)
{
	Spellers::iterator it = spellers_.find(word.lang_code());
	if (it != spellers_.end())
		aspell_speller_add_to_personal(it->second.speller, word.word().c_str(), -1);
}


void ASpell::accept(WordLangTuple const & word)
{
	Spellers::iterator it = spellers_.find(word.lang_code());
	if (it != spellers_.end())
		aspell_speller_add_to_session(it->second.speller, word.word().c_str(), -1);
}


string const ASpell::nextMiss()
{
	char const * str = 0;

	if (els)
		str = aspell_string_enumeration_next(els);
	if (str)
		return str;
	return "";
}


string const ASpell::error()
{
	char const * err = 0;

	if (spell_error_object && aspell_error_number(spell_error_object) != 0) {
		err = aspell_error_message(spell_error_object);
	}

	if (err)
		return err;
	return "";
}

#endif // USE_ASPELL
