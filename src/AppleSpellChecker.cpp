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

	SpellChecker::Result toResult(SpellCheckResult status);
	string toString(SpellCheckResult status);

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


SpellChecker::Result AppleSpellChecker::Private::toResult(SpellCheckResult status)
{
	return status == SPELL_CHECK_FAILED ? UNKNOWN_WORD :
		status == SPELL_CHECK_LEARNED ? LEARNED_WORD : WORD_OK ;
}


string AppleSpellChecker::Private::toString(SpellCheckResult status)
{
	return status == SPELL_CHECK_FAILED ? "FAILED" :
		 status == SPELL_CHECK_LEARNED ? "LEARNED" : "OK";
}


SpellChecker::Result AppleSpellChecker::check(WordLangTuple const & word)
{
	string const word_str = to_utf8(word.word());
	SpellCheckResult result =
		AppleSpeller_check(d->speller,
			word_str.c_str(), word.lang()->code().c_str());
	LYXERR(Debug::GUI, "spellCheck: \"" <<
		   word.word() << "\" = " << d->toString(result) <<
		   ", lang = " << word.lang()->code()) ;
	return d->toResult(result);
}


void AppleSpellChecker::advanceChangeNumber()
{
	nextChangeNumber();
}


// add to personal dictionary
void AppleSpellChecker::insert(WordLangTuple const & word)
{
	string const word_str = to_utf8(word.word());
	AppleSpeller_learn(d->speller, word_str.c_str());
	LYXERR(Debug::GUI, "learn word: \"" << word.word() << "\"") ;
	advanceChangeNumber();
}


// remove from personal dictionary
void AppleSpellChecker::remove(WordLangTuple const & word)
{
	string const word_str = to_utf8(word.word());
	AppleSpeller_unlearn(d->speller, word_str.c_str());
	LYXERR(Debug::GUI, "unlearn word: \"" << word.word() << "\"") ;
	advanceChangeNumber();
}


// ignore for session
void AppleSpellChecker::accept(WordLangTuple const & word)
{
	string const word_str = to_utf8(word.word());
	AppleSpeller_ignore(d->speller, word_str.c_str());
	advanceChangeNumber();
}


void AppleSpellChecker::suggest(WordLangTuple const & wl,
	docstring_list & suggestions)
{
	suggestions.clear();
	string const word_str = to_utf8(wl.word());
	size_t num = AppleSpeller_makeSuggestion(d->speller, word_str.c_str(), wl.lang()->code().c_str());
	for (size_t i = 0; i < num; i++) {
		char const * next = AppleSpeller_getSuggestion(d->speller, i);
		if (!next) break;
		suggestions.push_back(from_utf8(next));
	}
}


bool AppleSpellChecker::hasDictionary(Language const * lang) const
{
	return AppleSpeller_hasLanguage(d->speller,lang->code().c_str());
}


int AppleSpellChecker::numMisspelledWords() const
{
	return AppleSpeller_numMisspelledWords(d->speller);
}


void AppleSpellChecker::misspelledWord(int index, int & start, int & length) const
{
	AppleSpeller_misspelledWord(d->speller, index, &start, &length);
}


docstring const AppleSpellChecker::error()
{
	return docstring();
}


} // namespace lyx
