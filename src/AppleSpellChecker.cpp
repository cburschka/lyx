/**
 * \file AppleSpellChecker.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Stephan Witt
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "AppleSpellChecker.h"
#include "WordLangTuple.h"

#include "support/lassert.h"
#include "support/debug.h"
#include "support/docstring_list.h"
#include "support/AppleSpeller.h"

using namespace std;
using namespace lyx::support;

namespace lyx {

struct AppleSpellChecker::Private
{
	Private();

	~Private();

	/// the speller
	AppleSpeller speller;
};


AppleSpellChecker::Private::Private()
{
	speller = newAppleSpeller();
}


AppleSpellChecker::Private::~Private()
{
	freeAppleSpeller(speller);
	speller = 0;
}


AppleSpellChecker::AppleSpellChecker(): d(new Private)
{
}


AppleSpellChecker::~AppleSpellChecker()
{
	delete d;
}


SpellChecker::Result AppleSpellChecker::check(WordLangTuple const & word)
{
	string const word_str = to_utf8(word.word());
	int const word_ok = checkAppleSpeller(d->speller, word_str.c_str(), word.lang()->code().c_str());
	return (word_ok) ? OK : UNKNOWN_WORD;
}


// add to personal dictionary
void AppleSpellChecker::insert(WordLangTuple const & word)
{
	string const word_str = to_utf8(word.word());
	learnAppleSpeller(d->speller, word_str.c_str(), word.lang()->code().c_str());
}


// ignore for session
void AppleSpellChecker::accept(WordLangTuple const & word)
{
	string const word_str = to_utf8(word.word());
	ignoreAppleSpeller(d->speller, word_str.c_str(), word.lang()->code().c_str());
}


void AppleSpellChecker::suggest(WordLangTuple const & wl,
	docstring_list & suggestions)
{
	suggestions.clear();
	string const word_str = to_utf8(wl.word());
	size_t num = makeSuggestionAppleSpeller(d->speller, word_str.c_str(), wl.lang()->code().c_str());
	for (size_t i = 0; i < num; i++) {
		char const * next = getSuggestionAppleSpeller(d->speller, i);
		if (!next) break;
		suggestions.push_back(from_utf8(next));
	}
}


bool AppleSpellChecker::hasDictionary(Language const * lang) const
{
	return hasLanguageAppleSpeller(d->speller,lang->code().c_str());
}


docstring const AppleSpellChecker::error()
{
	return docstring();
}


} // namespace lyx
