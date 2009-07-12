/**
 * \file HunspellSpellChecker.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Abdelrazak Younes
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "HunspellSpellChecker.h"

#include "LyXRC.h"
#include "WordLangTuple.h"

#include "support/lassert.h"
#include "support/debug.h"

#include <hunspell/hunspell.hxx>

#include <map>
#include <string>

using namespace std;

namespace lyx {

namespace {
typedef map<std::string, Hunspell *> Spellers;
}

class HunspellSpellChecker::Private
{
	/// the spellers
	Spellers spellers_;
};


HunspellSpellChecker::HunspellSpellChecker(): d(new Private)
{
}


HunspellSpellChecker::~HunspellSpellChecker()
{
	delete d;
}


SpellChecker::Result HunspellSpellChecker::check(WordLangTuple const & word)
{
	return OK;
}


void HunspellSpellChecker::insert(WordLangTuple const & word)
{
}


void HunspellSpellChecker::accept(WordLangTuple const & word)
{
}


docstring const HunspellSpellChecker::nextMiss()
{
	return docstring();
}


docstring const HunspellSpellChecker::error()
{
	return docstring();
}


} // namespace lyx
