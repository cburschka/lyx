/**
 * \file HunspellChecker.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Abdelrazak Younes
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "HunspellChecker.h"

#include "LyXRC.h"
#include "WordLangTuple.h"

#include "support/lassert.h"
#include "support/debug.h"
#include "support/docstring_list.h"

#include <hunspell/hunspell.hxx>

#include <map>
#include <string>

// FIXME (Abdel): I still got linking problems but if anybody wants
// to try, defines this to 1.
#define TRY_HUNSPELL 0

using namespace std;

namespace lyx {

namespace {

typedef map<std::string, Hunspell *> Spellers;

} // anon namespace

struct HunspellChecker::Private
{
	Private() {}

	~Private();

	Hunspell * addSpeller(string const & lang);
	Hunspell * speller(string const & lang);

	/// the spellers
	Spellers spellers_;
};


HunspellChecker::Private::~Private()
{
#if TRY_HUNSPELL
	Spellers::iterator it = spellers_.begin();
	Spellers::iterator end = spellers_.end();

	for (; it != end; ++it) {
		delete it->second;
	}
#endif
}


Hunspell * HunspellChecker::Private::addSpeller(string const & lang)
{
	// FIXME: not implemented!

	// FIXME: We should we indicate somehow that this language is not
	// supported.
	return 0;
}


Hunspell * HunspellChecker::Private::speller(string const & lang)
{
	Spellers::iterator it = spellers_.find(lang);
	if (it != spellers_.end())
		return it->second;
	
	return addSpeller(lang);
}


HunspellChecker::HunspellChecker(): d(new Private)
{
}


HunspellChecker::~HunspellChecker()
{
	delete d;
}


SpellChecker::Result HunspellChecker::check(WordLangTuple const & wl)
{
	string const word_to_check = to_utf8(wl.word());
#if TRY_HUNSPELL
	Hunspell * h = d->speller(wl.lang_code());
	int info;
	if (h->spell(word_to_check.c_str(), &info))
		return OK;
	// FIXME: What to do with that?
	switch (info) {
	case SPELL_COMPOUND:
	case SPELL_FORBIDDEN:
	default:
		return UNKNOWN_WORD;
	}
	return UNKNOWN_WORD;
#endif
	return OK;
}


void HunspellChecker::insert(WordLangTuple const & wl)
{
	string const word_to_check = to_utf8(wl.word());
#if TRY_HUNSPELL
	Hunspell * h = d->speller(wl.lang_code());
	h->add(word_to_check.c_str());
#endif
}


void HunspellChecker::accept(WordLangTuple const & word)
{
	// FIXME: not implemented!
}


void HunspellChecker::suggest(WordLangTuple const & wl,
	docstring_list & suggestions)
{
	suggestions.clear();
	string const word_to_check = to_utf8(wl.word());
#if TRY_HUNSPELL
	Hunspell * h = d->speller(wl.lang_code());
	char *** suggestion_list = 0;
	int const suggestion_number = h->suggest(suggestion_list, word_to_check.c_str());
	if (suggestion_number == 0)
		return;
	h->free_list(suggestion_list, suggestion_number);
#endif
}


docstring const HunspellChecker::error()
{
	return docstring();
}


} // namespace lyx
