/**
 * \file ASpell.cpp
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

#include <aspell.h>

#include "ASpell_local.h"
#include "LyXRC.h"
#include "WordLangTuple.h"

#include "support/lassert.h"

using namespace std;

namespace lyx {

ASpell::ASpell(BufferParams const &, string const & lang,
	       string const & variety)
	: els(0), spell_error_object(0)
{
	addSpeller(lang, variety);
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


void ASpell::addSpeller(string const & lang, string const & variety)
{
	AspellConfig * config = new_aspell_config();
	// Aspell supports both languages and varieties (such as German
	// old vs. new spelling). The respective naming convention is
	// lang_REGION-variety (e.g. de_DE-alt).
	aspell_config_replace(config, "lang", lang.c_str());
	if (!variety.empty())
		aspell_config_replace(config, "variety", variety.c_str());
	// Set the encoding to utf-8.
	// aspell does also understand "ucs-4", so we would not need a
	// conversion in theory, but if this is used it expects all
	// char const * arguments to be a cast from  uint const *, and it
	// seems that this uint is not compatible with our char_type on some
	// platforms (cygwin, OS X). Therefore we use utf-8, that does
	// always work.
	aspell_config_replace(config, "encoding", "utf-8");
	if (lyxrc.isp_accept_compound)
		// Consider run-together words as legal compounds
		aspell_config_replace(config, "run-together", "true");
	else
		// Report run-together words as errors
		aspell_config_replace(config, "run-together", "false");
	AspellCanHaveError * err = new_aspell_speller(config);
	if (spell_error_object)
		delete_aspell_can_have_error(spell_error_object);
	spell_error_object = 0;

	if (aspell_error_number(err) == 0) {
		Speller m;
		m.speller = to_aspell_speller(err);
		m.config = config;
		spellers_[spellerID(lang, variety)] = m;
	} else {
		spell_error_object = err;
	}
}


ASpell::Result ASpell::check(WordLangTuple const & word)
{
	Result res = UNKNOWN_WORD;

	string const speller =
		spellerID(word.lang_code(), word.lang_variety());
	
	Spellers::iterator it = spellers_.find(speller);
	if (it == spellers_.end()) {
		addSpeller(word.lang_code(), word.lang_variety());
		it = spellers_.find(speller);
		// FIXME
		if (it == spellers_.end())
			return res;
	}

	AspellSpeller * m = it->second.speller;

	int const word_ok = aspell_speller_check(m, to_utf8(word.word()).c_str(), -1);
	LASSERT(word_ok != -1, /**/);

	if (word_ok)
		return OK;

	AspellWordList const * sugs =
		aspell_speller_suggest(m, to_utf8(word.word()).c_str(), -1);
	LASSERT(sugs != 0, /**/);
	els = aspell_word_list_elements(sugs);
	if (aspell_word_list_empty(sugs))
		res = UNKNOWN_WORD;
	else
		res = SUGGESTED_WORDS;

	return res;
}


void ASpell::insert(WordLangTuple const & word)
{
	Spellers::iterator it =
		spellers_.find(spellerID(word.lang_code(), word.lang_variety()));
	if (it != spellers_.end())
		aspell_speller_add_to_personal(
			it->second.speller, to_utf8(word.word()).c_str(), -1);
}


void ASpell::accept(WordLangTuple const & word)
{
	Spellers::iterator it =
		spellers_.find(spellerID(word.lang_code(), word.lang_variety()));
	if (it != spellers_.end())
		aspell_speller_add_to_session(
			it->second.speller, to_utf8(word.word()).c_str(), -1);
}


docstring const ASpell::nextMiss()
{
	char const * str = 0;

	if (els)
		str = aspell_string_enumeration_next(els);

	return (str ? from_utf8(str) : docstring());
}


docstring const ASpell::error()
{
	char const * err = 0;

	if (spell_error_object && aspell_error_number(spell_error_object) != 0)
		err = aspell_error_message(spell_error_object);

	// FIXME UNICODE: err is not in UTF8, but probably the locale encoding
	return (err ? from_utf8(err) : docstring());
}


string const ASpell::spellerID(string const & lang, string const & variety)
{
	if (variety.empty())
		return lang;
	return lang + "-" + variety;
}


} // namespace lyx
