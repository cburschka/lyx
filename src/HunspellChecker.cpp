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

#include <hunspell/hunspell.hxx>

#include <map>
#include <string>

using namespace std;

namespace lyx {

namespace {
typedef map<std::string, Hunspell *> Spellers;
}

class HunspellChecker::Private
{
	/// the spellers
	Spellers spellers_;
};


HunspellChecker::HunspellChecker(): d(new Private)
{
}


HunspellChecker::~HunspellChecker()
{
	delete d;
}


SpellChecker::Result HunspellChecker::check(WordLangTuple const & word)
{
	return OK;
}


void HunspellChecker::insert(WordLangTuple const & word)
{
}


void HunspellChecker::accept(WordLangTuple const & word)
{
}


docstring const HunspellChecker::nextMiss()
{
	return docstring();
}


docstring const HunspellChecker::error()
{
	return docstring();
}


} // namespace lyx
