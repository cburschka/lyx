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
#include "PersonalWordList.h"

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
typedef map<std::string, PersonalWordList *> LangPersonalWordList;

typedef vector<WordLangTuple> IgnoreList;

} // anon namespace


struct HunspellChecker::Private
{
	Private();
	~Private();

	void cleanCache();
	void setUserPath(std::string const & path);
	const string dictPath(int selector);
	bool haveLanguageFiles(string const & hpath);
	bool haveDictionary(Language const * lang, string & hpath);
	bool haveDictionary(Language const * lang);
	int numDictionaries() const;
	Hunspell * addSpeller(Language const * lang, string & hpath);
	Hunspell * addSpeller(Language const * lang);
	Hunspell * speller(Language const * lang);
	/// ignored words
	bool isIgnored(WordLangTuple const & wl) const;
	/// personal word list interface
	void remove(WordLangTuple const & wl);
	void insert(WordLangTuple const & wl);
	bool learned(WordLangTuple const & wl);
	/// the spellers
	Spellers spellers_;
	///
	IgnoreList ignored_;
	///
	LangPersonalWordList personal_;
	///
	std::string user_path_;

	/// the location below system/user directory
	/// there the aff+dic files lookup will happen
	const string dictDirectory(void) const { return "dicts"; }
	int maxLookupSelector(void) const { return 5; }
	const string HunspellDictionaryName(Language const * lang) {
		return lang->variety().empty() 
			? lang->code()
			: lang->code() + "-" + lang->variety();
	}
	const string myspellPackageDictDirectory(void) {
		return "/usr/share/myspell";
	}
	const string hunspellPackageDictDirectory(void) {
		return "/usr/share/hunspell";
	}
};


HunspellChecker::Private::Private()
{
	setUserPath(lyxrc.hunspelldir_path);
}


HunspellChecker::Private::~Private()
{
	cleanCache();
}


void HunspellChecker::Private::setUserPath(std::string const & path)
{
	if (user_path_ != lyxrc.hunspelldir_path) {
		cleanCache();
		user_path_ = path;
	}
}


void HunspellChecker::Private::cleanCache()
{
	Spellers::iterator it = spellers_.begin();
	Spellers::iterator end = spellers_.end();

	for (; it != end; ++it) {
		delete it->second;
		it->second = 0;
	}

	LangPersonalWordList::const_iterator pdit = personal_.begin();
	LangPersonalWordList::const_iterator pdet = personal_.end();

	for (; pdit != pdet; ++pdit) {
		if ( 0 == pdit->second)
			continue;
		PersonalWordList * pd = pdit->second;
		pd->save();
		delete pd;
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
	case 4:
		return hunspellPackageDictDirectory();
	case 3:
		return myspellPackageDictDirectory();
	case 2:
		return addName(package().system_support().absFileName(),dictDirectory());
	case 1:
		return addName(package().user_support().absFileName(),dictDirectory());
	default:
		return user_path_;
	}
}


bool HunspellChecker::Private::haveDictionary(Language const * lang, string & hpath)
{
	if (hpath.empty())
		return false;

	LYXERR(Debug::FILES, "check hunspell path: " << hpath
				<< " for language " << (lang ? lang->lang() : "NULL" ));

	string h_path = addName(hpath, HunspellDictionaryName(lang));
	// first we try lang code+variety
	if (haveLanguageFiles(h_path)) {
		LYXERR(Debug::FILES, "  found " << h_path);
		hpath = h_path;
		return true;
	}
	// another try with code, '_' replaced by '-'
	h_path = addName(hpath, subst(lang->code(), '_', '-'));
	if (!haveLanguageFiles(h_path))
		return false;
	LYXERR(Debug::FILES, "  found " << h_path);
	hpath = h_path;
	return true;
}


bool HunspellChecker::Private::haveDictionary(Language const * lang)
{
	bool result = false;

	setUserPath(lyxrc.hunspelldir_path);
	for (int p = 0; !result && p < maxLookupSelector(); ++p) {
		string lpath = dictPath(p);
		result = haveDictionary(lang, lpath);
	}
	// FIXME: if result is false... 
	// we should indicate somehow that this language is not
	// supported, probably by popping a warning. But we'll need to
	// remember which warnings we've issued.
	return result;
}


Hunspell * HunspellChecker::Private::speller(Language const * lang)
{
	setUserPath(lyxrc.hunspelldir_path);
	Spellers::iterator it = spellers_.find(lang->lang());
	if (it != spellers_.end())
		return it->second;
	return addSpeller(lang);
}


Hunspell * HunspellChecker::Private::addSpeller(Language const * lang,string & path)
{
	if (!haveDictionary(lang, path)) {
		spellers_[lang->lang()] = 0;
		return 0;
	}

	FileName const affix(path + ".aff");
	FileName const dict(path + ".dic");
	Hunspell * h = new Hunspell(affix.absFileName().c_str(), dict.absFileName().c_str());
	LYXERR(Debug::FILES, "Hunspell speller for langage " << lang << " at " << dict << " found");
	spellers_[lang->lang()] = h;
	return h;
}


Hunspell * HunspellChecker::Private::addSpeller(Language const * lang)
{
	Hunspell * h = 0;
	for (int p = 0; p < maxLookupSelector() && 0 == h; ++p) {
		string lpath = dictPath(p);
		h = addSpeller(lang, lpath);
	}
	if (h) {
		string const encoding = h->get_dic_encoding();
		PersonalWordList * pd = new PersonalWordList(lang->lang());
		pd->load();
		personal_[lang->lang()] = pd;
		docstring_list::const_iterator it = pd->begin();
		docstring_list::const_iterator et = pd->end();
		for (; it != et; ++it) {
			string const word_to_add = to_iconv_encoding(*it, encoding);
			h->add(word_to_add.c_str());
		}
	}
	return h;
}


int HunspellChecker::Private::numDictionaries() const
{
	int result = 0;
	Spellers::const_iterator it = spellers_.begin();
	Spellers::const_iterator et = spellers_.end();

	for (; it != et; ++it)
		result += it->second != 0;
	return result;
}


bool HunspellChecker::Private::isIgnored(WordLangTuple const & wl) const
{
	IgnoreList::const_iterator it = ignored_.begin();
	for (; it != ignored_.end(); ++it) {
		if (it->lang()->code() != wl.lang()->code())
			continue;
		if (it->word() == wl.word())
			return true;
	}
	return false;
}

/// personal word list interface
void HunspellChecker::Private::remove(WordLangTuple const & wl)
{
	Hunspell * h = speller(wl.lang());
	if (!h)
		return;
	string const encoding = h->get_dic_encoding();
	string const word_to_check = to_iconv_encoding(wl.word(), encoding);
	h->remove(word_to_check.c_str());
	PersonalWordList * pd = personal_[wl.lang()->lang()];
	if (!pd)
		return;
	pd->remove(wl.word());
}


void HunspellChecker::Private::insert(WordLangTuple const & wl)
{
	Hunspell * h = speller(wl.lang());
	if (!h)
		return;
	string const encoding = h->get_dic_encoding();
	string const word_to_check = to_iconv_encoding(wl.word(), encoding);
	h->add(word_to_check.c_str());
	PersonalWordList * pd = personal_[wl.lang()->lang()];
	if (!pd)
		return;
	pd->insert(wl.word());
}


bool HunspellChecker::Private::learned(WordLangTuple const & wl)
{
	PersonalWordList * pd = personal_[wl.lang()->lang()];
	if (!pd)
		return false;
	return pd->exists(wl.word());
}


HunspellChecker::HunspellChecker()
	: d(new Private)
{}


HunspellChecker::~HunspellChecker()
{
	delete d;
}


SpellChecker::Result HunspellChecker::check(WordLangTuple const & wl)
{
	if (d->isIgnored(wl))
		return WORD_OK;

	Hunspell * h = d->speller(wl.lang());
	if (!h)
		return NO_DICTIONARY;
	int info;

	string const encoding = h->get_dic_encoding();
	string const word_to_check = to_iconv_encoding(wl.word(), encoding);

	LYXERR(Debug::GUI, "spellCheck: \"" <<
		   wl.word() << "\", lang = " << wl.lang()->lang()) ;
	if (h->spell(word_to_check.c_str(), &info))
		return d->learned(wl) ? LEARNED_WORD : WORD_OK;

	if (info & SPELL_COMPOUND) {
		// FIXME: What to do with that?
		LYXERR(Debug::GUI, "Hunspell compound word found " << word_to_check);
	}
	if (info & SPELL_FORBIDDEN) {
		// This was removed from personal dictionary
		LYXERR(Debug::GUI, "Hunspell explicit forbidden word found " << word_to_check);
	}

	return UNKNOWN_WORD;
}


void HunspellChecker::advanceChangeNumber()
{
	nextChangeNumber();
}


void HunspellChecker::insert(WordLangTuple const & wl)
{
	d->insert(wl);
	LYXERR(Debug::GUI, "learn word: \"" << wl.word() << "\"") ;
	advanceChangeNumber();
}


void HunspellChecker::remove(WordLangTuple const & wl)
{
	d->remove(wl);
	LYXERR(Debug::GUI, "unlearn word: \"" << wl.word() << "\"") ;
	advanceChangeNumber();
}


void HunspellChecker::accept(WordLangTuple const & wl)
{
	d->ignored_.push_back(wl);
	LYXERR(Debug::GUI, "ignore word: \"" << wl.word() << "\"") ;
	advanceChangeNumber();
}


void HunspellChecker::suggest(WordLangTuple const & wl,
	docstring_list & suggestions)
{
	suggestions.clear();
	Hunspell * h = d->speller(wl.lang());
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


void HunspellChecker::stem(WordLangTuple const & wl,
	docstring_list & suggestions)
{
	suggestions.clear();
	Hunspell * h = d->speller(wl.lang());
	if (!h)
		return;
	string const encoding = h->get_dic_encoding();
	string const word_to_check = to_iconv_encoding(wl.word(), encoding);
	char ** suggestion_list;
	int const suggestion_number = h->stem(&suggestion_list, word_to_check.c_str());
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
	return d->haveDictionary(lang);
}


int HunspellChecker::numDictionaries() const
{
	return d->numDictionaries();
}


docstring const HunspellChecker::error()
{
	return docstring();
}


} // namespace lyx
