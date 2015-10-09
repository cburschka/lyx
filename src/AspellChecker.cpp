/**
 * \file AspellChecker.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Kevin Atkinson
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "AspellChecker.h"
#include "PersonalWordList.h"

#include "LyXRC.h"
#include "WordLangTuple.h"

#include "support/lassert.h"
#include "support/debug.h"
#include "support/lstrings.h"
#include "support/docstring_list.h"

#include "support/filetools.h"
#include "support/Package.h"
#include "support/FileName.h"
#include "support/PathChanger.h"

#include <aspell.h>

#include <map>
#include <string>

using namespace std;
using namespace lyx::support;

namespace lyx {

namespace {

struct Speller {
	AspellConfig * config;
	AspellCanHaveError * e_speller;
	bool accept_compound;
	docstring_list ignored_words_;
};

typedef std::map<std::string, Speller> Spellers;
typedef map<std::string, PersonalWordList *> LangPersonalWordList;

} // anon namespace

struct AspellChecker::Private
{
	Private()
	{}

	~Private();

	/// add a speller of the given language and variety
	AspellSpeller * addSpeller(Language const * lang);

	///
	AspellSpeller * speller(Language const * lang);

	bool isValidDictionary(AspellConfig * config,
			string const & lang, string const & variety);
	int numDictionaries() const;
	bool checkAspellData(AspellConfig * config,
		string const & basepath, string const & datapath, string const & dictpath,
		string const & lang, string const & variety);
	AspellConfig * getConfig(string const & lang, string const & variety);

	string toAspellWord(docstring const & word) const;

	SpellChecker::Result check(AspellSpeller * m,
		WordLangTuple const & word) const;

	void initSessionDictionary(Speller const & speller, PersonalWordList * pd);
	void addToSession(AspellCanHaveError * speller, docstring const & word);
	void insert(WordLangTuple const & word);
	void remove(WordLangTuple const & word);
	bool learned(WordLangTuple const & word);

	void accept(Speller & speller, WordLangTuple const & word);

	/// the spellers
	Spellers spellers_;

	LangPersonalWordList personal_;

	/// the location below system/user directory
	/// there the rws files lookup will happen
	const string dictDirectory(void)
	{
		return "dicts";
	}
	/// there the dat+cmap files lookup will happen
	const string dataDirectory(void)
	{
		return "data";
	}
	/// os package directory constants
	/// macports on Mac OS X or
	/// aspell rpms on Linux
	const string osPackageBase(void)
	{
#ifdef USE_MACOSX_PACKAGING
		return "/opt/local";
#else
		return "/usr";
#endif
	}
	const string osPackageDictDirectory(void)
	{
#ifdef USE_MACOSX_PACKAGING
		return "/share/aspell";
#else
		return "/lib/aspell-0.60";
#endif
	}
	const string osPackageDataDirectory(void)
	{
		return "/lib/aspell-0.60";
	}

};


AspellChecker::Private::~Private()
{
	Spellers::iterator it = spellers_.begin();
	Spellers::iterator end = spellers_.end();

	for (; it != end; ++it) {
		if (it->second.e_speller) {
			AspellSpeller * speller = to_aspell_speller(it->second.e_speller);
			aspell_speller_save_all_word_lists(speller);
			delete_aspell_can_have_error(it->second.e_speller);
		}
		delete_aspell_config(it->second.config);
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


bool AspellChecker::Private::isValidDictionary(AspellConfig * config,
		string const & lang, string const & variety)
{
	bool have = false;
	// code taken from aspell's list-dicts example
	// the returned pointer should _not_ need to be deleted
	AspellDictInfoList * dlist = get_aspell_dict_info_list(config);
	AspellDictInfoEnumeration * dels = aspell_dict_info_list_elements(dlist);
	const AspellDictInfo * entry;

	while (0 != (entry = aspell_dict_info_enumeration_next(dels))) {
		LYXERR(Debug::DEBUG, "aspell dict:"
			<< " name="    << entry->name
			<< ",code="    << entry->code
			<< ",variety=" << entry->jargon);
		if (entry->code == lang && (variety.empty() || entry->jargon == variety)) {
			have = true;
			break;
		}
	}
	delete_aspell_dict_info_enumeration(dels);
	LYXERR(Debug::FILES, "aspell dictionary: " << lang << (have ? " yes" : " no"));
	return have;
}


bool AspellChecker::Private::checkAspellData(AspellConfig * config,
	string const & basepath, string const & datapath, string const & dictpath,
	string const & lang, string const & variety)
{
	FileName base(basepath);
	bool have_dict = base.isDirectory() ;

	if (have_dict) {
		FileName data(addPath(base.absFileName(), datapath));
		FileName dict(addPath(base.absFileName(), dictpath));
		have_dict = dict.isDirectory() && data.isDirectory();
		if (have_dict) {
			LYXERR(Debug::FILES, "aspell dict-dir: " << dict);
			LYXERR(Debug::FILES, "aspell data-dir: " << data);
			aspell_config_replace(config, "dict-dir", dict.absFileName().c_str());
			aspell_config_replace(config, "data-dir", data.absFileName().c_str());
			have_dict = isValidDictionary(config, lang, variety);
		}
	}
	return have_dict ;
}


AspellConfig * AspellChecker::Private::getConfig(string const & lang, string const & variety)
{
	AspellConfig * config = new_aspell_config();
	bool have_dict = false;
	string const sysdir = lyx::support::package().system_support().absFileName() ;
	string const userdir = lyx::support::package().user_support().absFileName() ;

	LYXERR(Debug::FILES, "aspell user dir: " << userdir);
	have_dict = checkAspellData(config, userdir, dataDirectory(), dictDirectory(), lang, variety);
	if (!have_dict) {
		LYXERR(Debug::FILES, "aspell sysdir dir: " << sysdir);
		have_dict = checkAspellData(config, sysdir, dataDirectory(), dictDirectory(), lang, variety);
	}
	if (!have_dict) {
		// check for package data of OS installation
		checkAspellData(config, osPackageBase(), osPackageDataDirectory(), osPackageDictDirectory(), lang, variety);
	}
	return config ;
}


void AspellChecker::Private::addToSession(AspellCanHaveError * speller, docstring const & word)
{
	string const word_to_add = toAspellWord(word);
	if(1 != aspell_speller_add_to_session(to_aspell_speller(speller), word_to_add.c_str(), -1))
		LYXERR(Debug::GUI, "aspell add to session: " << aspell_error_message(speller));
}


void AspellChecker::Private::initSessionDictionary(
	Speller const & speller,
	PersonalWordList * pd)
{
	AspellSpeller * aspell = to_aspell_speller(speller.e_speller);
	aspell_speller_clear_session(aspell);
	docstring_list::const_iterator it = pd->begin();
	docstring_list::const_iterator et = pd->end();
	for (; it != et; ++it) {
		addToSession(speller.e_speller, *it);
	}
	it = speller.ignored_words_.begin();
	et = speller.ignored_words_.end();
	for (; it != et; ++it) {
		addToSession(speller.e_speller, *it);
	}
}


AspellSpeller * AspellChecker::Private::addSpeller(Language const * lang)
{
	Speller m;
	string const code = lang->code();
	string const variety = lang->variety();
	m.config = getConfig(code, variety);
	// Aspell supports both languages and varieties (such as German
	// old vs. new spelling). The respective naming convention is
	// lang_REGION-variety (e.g. de_DE-alt).
	aspell_config_replace(m.config, "lang", code.c_str());
	if (!variety.empty())
		aspell_config_replace(m.config, "variety", variety.c_str());
	// Set the encoding to utf-8.
	// aspell does also understand "ucs-4", so we would not need a
	// conversion in theory, but if this is used it expects all
	// char const * arguments to be a cast from  uint const *, and it
	// seems that this uint is not compatible with our char_type on some
	// platforms (cygwin, OS X). Therefore we use utf-8, that does
	// always work.
	aspell_config_replace(m.config, "encoding", "utf-8");
	if (lyxrc.spellchecker_accept_compound)
		// Consider run-together words as legal compounds
		aspell_config_replace(m.config, "run-together", "true");
	else
		// Report run-together words as errors
		aspell_config_replace(m.config, "run-together", "false");

	m.accept_compound = lyxrc.spellchecker_accept_compound;
	m.e_speller = new_aspell_speller(m.config);
	if (aspell_error_number(m.e_speller) != 0) {
		// FIXME: We should indicate somehow that this language is not supported.
		LYXERR(Debug::FILES, "aspell error: " << aspell_error_message(m.e_speller));
		delete_aspell_can_have_error(m.e_speller);
		delete_aspell_config(m.config);
		m.config = 0;
		m.e_speller = 0;
	} else {
		PersonalWordList * pd = new PersonalWordList(lang->lang());
		pd->load();
		personal_[lang->lang()] = pd;
		initSessionDictionary(m, pd);
	}
	
	spellers_[lang->lang()] = m;
	return m.e_speller ? to_aspell_speller(m.e_speller) : 0;
}


AspellSpeller * AspellChecker::Private::speller(Language const * lang)
{
	Spellers::iterator it = spellers_.find(lang->lang());
	if (it != spellers_.end()) {
		Speller aspell = it->second;
		if (lyxrc.spellchecker_accept_compound != aspell.accept_compound) {
			// spell checker setting changed... adjust run-together
			aspell.accept_compound = lyxrc.spellchecker_accept_compound;
			if (aspell.accept_compound)
				// Consider run-together words as legal compounds
				aspell_config_replace(aspell.config, "run-together", "true");
			else
				// Report run-together words as errors
				aspell_config_replace(aspell.config, "run-together", "false");
			AspellCanHaveError * e_speller = aspell.e_speller;
			aspell.e_speller = new_aspell_speller(aspell.config);
			delete_aspell_speller(to_aspell_speller(e_speller));
			spellers_[lang->lang()] = aspell;
		}
		return to_aspell_speller(aspell.e_speller);
	}

	return addSpeller(lang);
}


int AspellChecker::Private::numDictionaries() const
{
	int result = 0;
	Spellers::const_iterator it = spellers_.begin();
	Spellers::const_iterator et = spellers_.end();

	for (; it != et; ++it) {
		Speller aspell = it->second;
		result += aspell.e_speller != 0;
	}
	return result;
}


string AspellChecker::Private::toAspellWord(docstring const & word) const
{
	size_t mpos;
	string word_str = to_utf8(word);
	while ((mpos = word_str.find('-')) != word_str.npos) {
		word_str.erase(mpos, 1);
	}
	return word_str;
}


SpellChecker::Result AspellChecker::Private::check(
	AspellSpeller * m, WordLangTuple const & word) 
	const
{
	SpellChecker::Result result = WORD_OK;
	docstring w1;
	LYXERR(Debug::GUI, "spellCheck: \"" <<
		   word.word() << "\", lang = " << word.lang()->lang()) ;
	docstring rest = split(word.word(), w1, '-');
	for (; result == WORD_OK;) {
		string const word_str = toAspellWord(w1);
		int const word_ok = aspell_speller_check(m, word_str.c_str(), -1);
		LASSERT(word_ok != -1, return UNKNOWN_WORD);
		result = (word_ok) ? WORD_OK : UNKNOWN_WORD;
		if (rest.empty())
			break;
		rest = split(rest,w1,'-');
	}
	if (result == WORD_OK)
		return result;
	string const word_str = toAspellWord(word.word());
	int const word_ok = aspell_speller_check(m, word_str.c_str(), -1);
	LASSERT(word_ok != -1, return UNKNOWN_WORD);
	return (word_ok) ? WORD_OK : UNKNOWN_WORD;
}

void AspellChecker::Private::accept(Speller & speller, WordLangTuple const & word)
{
	speller.ignored_words_.push_back(word.word());
}


/// personal word list interface
void AspellChecker::Private::remove(WordLangTuple const & word)
{
	PersonalWordList * pd = personal_[word.lang()->lang()];
	if (!pd)
		return;
	pd->remove(word.word());
	Spellers::iterator it = spellers_.find(word.lang()->lang());
	if (it != spellers_.end()) {
		initSessionDictionary(it->second, pd);
	}
}

		
void AspellChecker::Private::insert(WordLangTuple const & word)
{
	Spellers::iterator it = spellers_.find(word.lang()->lang());
	if (it != spellers_.end()) {
		addToSession(it->second.e_speller, word.word());
		PersonalWordList * pd = personal_[word.lang()->lang()];
		if (!pd)
			return;
		pd->insert(word.word());
	}
}

bool AspellChecker::Private::learned(WordLangTuple const & word)
{
	PersonalWordList * pd = personal_[word.lang()->lang()];
	if (!pd)
		return false;
	return pd->exists(word.word());
}


AspellChecker::AspellChecker()
	: d(new Private)
{}


AspellChecker::~AspellChecker()
{
	delete d;
}


SpellChecker::Result AspellChecker::check(WordLangTuple const & word)
{
  
	AspellSpeller * m = d->speller(word.lang());

	if (!m)
		return NO_DICTIONARY;

	if (word.word().empty())
		// MSVC compiled Aspell doesn't like it.
		return WORD_OK;

	SpellChecker::Result rc = d->check(m, word);
	return (rc == WORD_OK && d->learned(word)) ? LEARNED_WORD : rc;
}


void AspellChecker::advanceChangeNumber()
{
	nextChangeNumber();
}


void AspellChecker::insert(WordLangTuple const & word)
{
	d->insert(word);
	advanceChangeNumber();
}


void AspellChecker::accept(WordLangTuple const & word)
{
	Spellers::iterator it = d->spellers_.find(word.lang()->lang());
	if (it != d->spellers_.end()) {
		d->addToSession(it->second.e_speller, word.word());
		d->accept(it->second, word);
		advanceChangeNumber();
	}
}


void AspellChecker::suggest(WordLangTuple const & wl,
	docstring_list & suggestions)
{
	suggestions.clear();
	AspellSpeller * m = d->speller(wl.lang());

	if (!m)
		return;

	string const word = d->toAspellWord(wl.word());
	AspellWordList const * sugs =
		aspell_speller_suggest(m, word.c_str(), -1);
	LASSERT(sugs != 0, return);
	AspellStringEnumeration * els = aspell_word_list_elements(sugs);
	if (!els || aspell_word_list_empty(sugs))
		return;

	for (;;) {
		char const * str = aspell_string_enumeration_next(els);
		if (!str)
			break;
		suggestions.push_back(from_utf8(str));
	}

	delete_aspell_string_enumeration(els);
}


void AspellChecker::remove(WordLangTuple const & word)
{
	d->remove(word);
	advanceChangeNumber();
}


bool AspellChecker::hasDictionary(Language const * lang) const
{
	bool have = false;
	Spellers::iterator it = d->spellers_.begin();
	Spellers::iterator end = d->spellers_.end();

	if (lang) {
		for (; it != end && !have; ++it) {
			have = it->second.config && d->isValidDictionary(it->second.config, lang->code(), lang->variety());
		}
		if (!have) {
			AspellConfig * config = d->getConfig(lang->code(), lang->variety());
			have = d->isValidDictionary(config, lang->code(), lang->variety());
			delete_aspell_config(config);
		}
	}
	return have;
}


int AspellChecker::numDictionaries() const
{
	return d->numDictionaries();
}
	
	
docstring const AspellChecker::error()
{
	Spellers::iterator it = d->spellers_.begin();
	Spellers::iterator end = d->spellers_.end();
	char const * err = 0;

	for (; it != end && 0 == err; ++it) {
		if (it->second.e_speller && aspell_error_number(it->second.e_speller) != 0)
			err = aspell_error_message(it->second.e_speller);
	}

	// FIXME UNICODE: err is not in UTF8, but probably the locale encoding
	return (err ? from_utf8(err) : docstring());
}


} // namespace lyx
