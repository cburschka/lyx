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
#include "support/Package.h"
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

	const string dictPath(int selector);
	bool haveLanguageFiles(string const & hpath);
	bool haveDictionary(string const & lang, string & hpath);
	bool haveDictionary(string const & lang);
	Hunspell * addSpeller(string const & lang, string & hpath);
	Hunspell * addSpeller(string const & lang);
	Hunspell * speller(string const & lang);
	/// ignored words
	bool isIgnored(WordLangTuple const & wl) const;

	/// the spellers
	Spellers spellers_;
	///
	IgnoreList ignored_;

	/// the location below system/user directory
	/// there the aff+dic files lookup will happen
	const string dictDirectory(void) const { return "dict"; }
	int maxLookupSelector(void) const { return 3; }
};


HunspellChecker::Private::~Private()
{
	Spellers::iterator it = spellers_.begin();
	Spellers::iterator end = spellers_.end();

	for (; it != end; ++it) {
		if ( 0 != it->second) delete it->second;
	}
}


bool HunspellChecker::Private::haveLanguageFiles(string const & hpath)
{
	FileName const affix(hpath + ".aff");
	FileName const dict(hpath + ".dic");
	return affix.isReadableFile() && dict.isReadableFile();
}


const string HunspellChecker::Private::dictPath(int selector)
{
	switch (selector) {
	case 2:
		return addName(lyx::support::package().system_support().absFileName(),dictDirectory());
		break;
	case 1:
		return addName(lyx::support::package().user_support().absFileName(),dictDirectory());
		break;
	default:
		return lyxrc.hunspelldir_path;
	}
}


bool HunspellChecker::Private::haveDictionary(string const & lang, string & hpath)
{
	if (hpath.empty()) {
		return false;
	}

	LYXERR(Debug::FILES, "check hunspell path: " << hpath << " for language " << lang);
	string h_path = addName(hpath, lang);
	if (!haveLanguageFiles(h_path)) {
		// try with '_' replaced by '-'
		h_path = addName(hpath, subst(lang, '_', '-'));
		if (!haveLanguageFiles(h_path)) {
			// FIXME: We should indicate somehow that this language is not
			// supported, probably by popping a warning. But we'll need to
			// remember which warnings we've issued.
			return false;
		}
	}
	hpath = h_path;
	return true;
}


bool HunspellChecker::Private::haveDictionary(string const & lang)
{
	bool result = false;
	for ( int p = 0; !result && p < maxLookupSelector(); p++ ) {
		string lpath = dictPath(p);
		result = haveDictionary(lang, lpath);
	}
	return result;
}


Hunspell * HunspellChecker::Private::speller(string const & lang)
{
	Spellers::iterator it = spellers_.find(lang);
	if (it != spellers_.end())
		return it->second;
	
	return addSpeller(lang);
}


Hunspell * HunspellChecker::Private::addSpeller(string const & lang,string & path)
{
	if (!haveDictionary(lang, path)) {
		spellers_[lang] = 0;
		return 0;
	}

	FileName const affix(path + ".aff");
	FileName const dict(path + ".dic");
	Hunspell * h = new Hunspell(affix.absFileName().c_str(), dict.absFileName().c_str());
	LYXERR(Debug::FILES, "Hunspell speller for langage " << lang << " at " << dict << " found");
	spellers_[lang] = h;
	return h;
}


Hunspell * HunspellChecker::Private::addSpeller(string const & lang)
{
	Hunspell * h = 0;
	for ( int p = 0; p < maxLookupSelector() && 0 == h; p++ ) {
		string lpath = dictPath(p);
		h = addSpeller(lang, lpath);
	}
	return h;
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
		return WORD_OK;

	Hunspell * h = d->speller(wl.lang()->code());
	if (!h)
		return WORD_OK;
	int info;

	string const encoding = h->get_dic_encoding();
	string const word_to_check = to_iconv_encoding(wl.word(), encoding);
	
	if (h->spell(word_to_check.c_str(), &info))
		return WORD_OK;

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
	Hunspell * h = d->speller(wl.lang()->code());
	if (!h)
		return;
	string const encoding = h->get_dic_encoding();
	string const word_to_check = to_iconv_encoding(wl.word(), encoding);
	char ** suggestion_list;
	int const suggestion_number = h->suggest(&suggestion_list, word_to_check.c_str());
	if (suggestion_number <= 0)
		return;
	for (int i = 0; i != suggestion_number; ++i)
		suggestions.push_back(from_iconv_encoding(suggestion_list[i], encoding));
	h->free_list(&suggestion_list, suggestion_number);
}


bool HunspellChecker::hasDictionary(Language const * lang) const
{
	if (!lang)
		return false;
	return (d->haveDictionary(lang->code()));
}


docstring const HunspellChecker::error()
{
	return docstring();
}


} // namespace lyx
