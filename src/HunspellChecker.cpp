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

#include "support/debug.h"
#include "support/docstring_list.h"
#include "support/FileName.h"
#include "support/gettext.h"
#include "support/lassert.h"
#include "support/os.h"

#include <hunspell/hunspell.hxx>

#include <map>
#include <string>

using namespace std;
using namespace lyx::support;
using namespace lyx::support::os;

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
	Spellers::iterator it = spellers_.begin();
	Spellers::iterator end = spellers_.end();

	for (; it != end; ++it) {
		delete it->second;
	}
}


Hunspell * HunspellChecker::Private::addSpeller(string const & lang)
{
	string hunspell_path = external_path(lyxrc.hunspelldir_path);
	LYXERR(Debug::FILES, "hunspell path: " << hunspell_path);
	if (hunspell_path.empty())
		return false;

	hunspell_path += "/" + lang;
	// replace '_' with '-' as this is the convention used by hunspell.
	hunspell_path[hunspell_path.size() - 3] = '-';
	FileName const affix(hunspell_path + ".aff");
	FileName const dict(hunspell_path + ".dic");
	if (!affix.isReadableFile()) {
		// FIXME: We should indicate somehow that this language is not
		// supported.
		LYXERR(Debug::FILES, "Hunspell affix file " << affix << " does not exist");
		return 0;
	}
	if (!dict.isReadableFile()) {
		LYXERR(Debug::FILES, "Hunspell dictionary file " << dict << " does not exist");
		return 0;
	}
	Hunspell * h = new Hunspell(affix.absFilename().c_str(), dict.absFilename().c_str());
	spellers_[lang] = h;
	return h;
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
	Hunspell * h = d->speller(wl.lang_code());
	if (!h)
		return OK;
	int info;
	if (h->spell(word_to_check.c_str(), &info))
		return OK;

	if (info & SPELL_COMPOUND) {
		// FIXME: What to do with that?
		LYXERR(Debug::FILES, "Hunspell compound word found " << word_to_check);
	}
	if (info & SPELL_FORBIDDEN) {
		// FIXME: What to do with that?
		LYXERR(Debug::FILES, "Hunspell explicit forbidden word found " << word_to_check);
	}

	return UNKNOWN_WORD;
}


void HunspellChecker::insert(WordLangTuple const & wl)
{
	string const word_to_check = to_utf8(wl.word());
	Hunspell * h = d->speller(wl.lang_code());
	if (!h)
		return;
	h->add(word_to_check.c_str());
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
	Hunspell * h = d->speller(wl.lang_code());
	if (!h)
		return;
	char *** suggestion_list = 0;

	// FIXME: Hunspell::suggest() crashes on Win/MSVC9
	return;

	int const suggestion_number = h->suggest(suggestion_list, word_to_check.c_str());
	if (suggestion_number == 0)
		return;
	h->free_list(suggestion_list, suggestion_number);
}


docstring const HunspellChecker::error()
{
	return docstring();
}


} // namespace lyx
