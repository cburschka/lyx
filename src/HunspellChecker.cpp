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

#include "frontends/alert.h"

#include "support/debug.h"
#include "support/docstring_list.h"
#include "support/filetools.h"
#include "support/FileName.h"
#include "support/gettext.h"
#include "support/lassert.h"
#include "support/lstrings.h"
#include "support/os.h"

#include <hunspell/hunspell.hxx>

#include <map>
#include <string>
#include <vector>

using namespace std;
using namespace lyx::support;
using namespace lyx::support::os;

namespace lyx {

namespace {

typedef map<std::string, Hunspell *> Spellers;
typedef vector<WordLangTuple> IgnoreList;

} // anon namespace

struct HunspellChecker::Private
{
	Private() {}

	~Private();

	Hunspell * addSpeller(string const & lang);
	Hunspell * speller(string const & lang);
	/// ignored words
	bool isIgnored(WordLangTuple const & wl) const;

	/// the spellers
	Spellers spellers_;
	///
	IgnoreList ignored_;
};


HunspellChecker::Private::~Private()
{
	Spellers::iterator it = spellers_.begin();
	Spellers::iterator end = spellers_.end();

	for (; it != end; ++it) {
		delete it->second;
	}
}


namespace {
bool haveLanguageFiles(string const & hpath)
{
	FileName const affix(hpath + ".aff");
	FileName const dict(hpath + ".dic");
	if (!affix.isReadableFile()) {
		// FIXME: We should indicate somehow that this language is not
		// supported.
		LYXERR(Debug::FILES, "Hunspell affix file " << affix << " does not exist");
		return false;
	}
	if (!dict.isReadableFile()) {
		LYXERR(Debug::FILES, "Hunspell dictionary file " << dict << " does not exist");
		return false;
	}
	return true;
}
}


Hunspell * HunspellChecker::Private::addSpeller(string const & lang)
{
	string hunspell_path = lyxrc.hunspelldir_path;
	LYXERR(Debug::FILES, "hunspell path: " << external_path(hunspell_path));
	if (hunspell_path.empty()) {
		// FIXME We'd like to issue a better error message here, but there seems
		// to be a problem about thread safety, or something of the sort. If
		// we issue the message using frontend::Alert, then the code comes
		// back through here while the box is waiting, and causes some kind
		// of crash. 
		static bool warned = false;
		if (!warned) {
			warned = true;
			LYXERR0("Hunspell path not set.");
			//frontend::Alert::error(_("Hunspell Path Not Found"), 
			//		_("You must set the Hunspell dictionary path in Tools>Preferences>Paths."));
		}
		return 0;
	}

	hunspell_path = external_path(addName(hunspell_path, lang));
	if (!haveLanguageFiles(hunspell_path)) {
		// try with '_' replaced by '-'
		hunspell_path = subst(hunspell_path, '_', '-');
		if (!haveLanguageFiles(hunspell_path)) {
			// FIXME: We should indicate somehow that this language is not
			// supported, probably by popping a warning. But we'll need to
			// remember which warnings we've issued.
			return 0;
		}
	}
	FileName const affix(hunspell_path + ".aff");
	FileName const dict(hunspell_path + ".dic");
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


bool HunspellChecker::Private::isIgnored(WordLangTuple const & wl) const
{
	IgnoreList::const_iterator it = ignored_.begin();
	for (; it != ignored_.end(); ++it) {
		if ((*it).lang()->code() != wl.lang()->code())
			continue;
		if ((*it).word() == wl.word())
			return true;
	}
	return false;
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
	if (d->isIgnored(wl))
		return OK;

	string const word_to_check = to_utf8(wl.word());
	Hunspell * h = d->speller(wl.lang()->code());
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
	Hunspell * h = d->speller(wl.lang()->code());
	if (!h)
		return;
	h->add(word_to_check.c_str());
}


void HunspellChecker::accept(WordLangTuple const & wl)
{
	d->ignored_.push_back(wl);
}


void HunspellChecker::suggest(WordLangTuple const & wl,
	docstring_list & suggestions)
{
	suggestions.clear();
	string const word_to_check = to_utf8(wl.word());
	Hunspell * h = d->speller(wl.lang()->code());
	if (!h)
		return;
	char ** suggestion_list;
	int const suggestion_number = h->suggest(&suggestion_list, word_to_check.c_str());
	if (suggestion_number <= 0)
		return;
	for (int i = 0; i != suggestion_number; ++i)
		suggestions.push_back(from_utf8(suggestion_list[i]));
	h->free_list(&suggestion_list, suggestion_number);
}


docstring const HunspellChecker::error()
{
	return docstring();
}


} // namespace lyx
