/**
 * \file Enchant.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Caolán McNamara
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "support/debug.h"

#include <enchant++.h>

#include "Enchant.h"
#include "LyXRC.h"
#include "WordLangTuple.h"

#include "support/lassert.h"

using namespace std;

namespace lyx {

Enchant::Enchant(BufferParams const &, string const & lang)
{
	addSpeller(lang);
}


Enchant::~Enchant()
{
	Spellers::iterator it = spellers_.begin();
	Spellers::iterator end = spellers_.end();

	for (; it != end; ++it) {
		delete it->second.speller;
	}
}


void Enchant::addSpeller(string const & lang)
{
	enchant::Broker * instance = enchant::Broker::instance();
	enchant::Dict * dict = instance->request_dict(lang);

	if (dict) {
		Speller m;
		m.speller = dict;
		spellers_[lang] = m;
	}
}


Enchant::Result Enchant::check(WordLangTuple const & word)
{
	Result res = UNKNOWN_WORD;

	Spellers::iterator it = spellers_.find(word.lang_code());
	if (it == spellers_.end()) {
		addSpeller(word.lang_code());
		it = spellers_.find(word.lang_code());
		// FIXME
		if (it == spellers_.end())
			return res;
	}

	enchant::Dict * m = it->second.speller;

	std::string utf8word(to_utf8(word.word()));

	if (m->check(utf8word))
		return OK;

	m->suggest(utf8word, els);
	if (els.empty())
		res = UNKNOWN_WORD;
	else
		res = SUGGESTED_WORDS;

	return res;
}


void Enchant::insert(WordLangTuple const & word)
{
	Spellers::iterator it = spellers_.find(word.lang_code());
	if (it != spellers_.end())
		it->second.speller->add(to_utf8(word.word()));
}


void Enchant::accept(WordLangTuple const & word)
{
	Spellers::iterator it = spellers_.find(word.lang_code());
	if (it != spellers_.end())
		it->second.speller->add_to_session(to_utf8(word.word()));
}


docstring const Enchant::nextMiss()
{
	docstring str;

	if (!els.empty()) {
		str = from_utf8(els.front());
		els.erase(els.begin());
	}

	return str;
}


docstring const Enchant::error()
{
	return docstring();
}


} // namespace lyx
