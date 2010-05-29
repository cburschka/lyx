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
#include "LyXRC.h"
#include "WordLangTuple.h"

#include "support/lassert.h"
#include "support/debug.h"
#include "support/docstring_list.h"

#include "support/Package.h"
#include "support/FileName.h"
#include "support/Path.h"

#include <aspell.h>

#include <map>
#include <string>

#ifdef __APPLE__

# ifndef ASPELL_FRAMEWORK
# define ASPELL_FRAMEWORK "Aspell.framework"
# endif
# ifndef ASPELL_FRAMEWORK_DATA
# define ASPELL_FRAMEWORK_DATA "/Resources/data"
# endif
# ifndef ASPELL_FRAMEWORK_DICT
# define ASPELL_FRAMEWORK_DICT "/Resources/dict"
# endif

# ifndef ASPELL_MACPORTS
# define ASPELL_MACPORTS "/opt/local"
# endif
# ifndef ASPELL_MACPORTS_DATA
# define ASPELL_MACPORTS_DATA "/lib/aspell-0.60"
# endif
# ifndef ASPELL_MACPORTS_DICT
# define ASPELL_MACPORTS_DICT "/share/aspell"
# endif

#endif /* __APPLE__ */

using namespace std;
using namespace lyx::support;

namespace lyx {

namespace {

struct Speller {
	///AspellSpeller * speller;
	AspellConfig * config;
	AspellCanHaveError * e_speller;
};

typedef std::map<std::string, Speller> Spellers;

} // anon namespace

struct AspellChecker::Private
{
	Private() {}

	~Private();

	/// add a speller of the given language and variety
	AspellSpeller * addSpeller(string const & lang,
				   string const & variety = string());

	///
	AspellSpeller * speller(string const & lang,
				string const & variety);

	/// create a unique ID from lang code and variety
	string const spellerID(string const & lang,
			       string const & variety);

	/// the spellers
	Spellers spellers_;

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
}


bool isValidDictionary(AspellConfig * config,
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


bool checkAspellData(AspellConfig * config,
	char const * basepath, char const * datapath, char const * dictpath,
	string const & lang, string const & variety)
{
	bool have_dict = false;
	FileName base(basepath);
	FileName data(base.absFileName() + datapath);
	FileName dict(base.absFileName() + dictpath);
	have_dict = dict.isDirectory() && data.isDirectory();
	if (have_dict) {
		aspell_config_replace(config, "dict-dir", dict.absFileName().c_str());
		aspell_config_replace(config, "data-dir", data.absFileName().c_str());
		LYXERR(Debug::FILES, "aspell dict: " << dict);
		have_dict = isValidDictionary(config, lang, variety);
	}
	return have_dict ;
}


AspellConfig * getConfig(string const & lang,
						   string const & variety)
{
	AspellConfig * config = new_aspell_config();
#ifdef __APPLE__
	char buf[2048] ;
	bool have_dict = false;
	char const * sysdir = lyx::support::package().system_support().absFileName().c_str() ;
	char const * userdir = lyx::support::package().user_support().absFileName().c_str() ;
	char const * framework = ASPELL_FRAMEWORK ;

	LYXERR(Debug::FILES, "aspell sysdir dir: " << sysdir);
	LYXERR(Debug::FILES, "aspell user dir: " << userdir);
	have_dict = checkAspellData(config, userdir, ASPELL_FRAMEWORK_DATA, ASPELL_FRAMEWORK_DICT, lang, variety);
	if (!have_dict && strlen(framework) && getPrivateFrameworkPathName(buf, sizeof(buf), framework)) {
		LYXERR(Debug::FILES, "aspell bundle path: " << buf);
		have_dict = checkAspellData(config, buf, ASPELL_FRAMEWORK_DATA, ASPELL_FRAMEWORK_DICT, lang, variety);
	}
	if (!have_dict) {
		// check for macports data
		have_dict = checkAspellData(config, ASPELL_MACPORTS, ASPELL_MACPORTS_DATA, ASPELL_MACPORTS_DICT, lang, variety);
	}
#endif
	return config ;
}


AspellSpeller * AspellChecker::Private::addSpeller(string const & lang,
						   string const & variety)
{
	Speller m;

	m.config = getConfig(lang, variety);
	// Aspell supports both languages and varieties (such as German
	// old vs. new spelling). The respective naming convention is
	// lang_REGION-variety (e.g. de_DE-alt).
	aspell_config_replace(m.config, "lang", lang.c_str());
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

	m.e_speller = new_aspell_speller(m.config);
	if (aspell_error_number(m.e_speller) != 0) {
		// FIXME: We should indicate somehow that this language is not supported.
		LYXERR(Debug::FILES, "aspell error: " << aspell_error_message(m.e_speller));
	}

	spellers_[spellerID(lang, variety)] = m;
	return to_aspell_speller(m.e_speller);
}


AspellSpeller * AspellChecker::Private::speller(string const & lang,
						string const & variety)
{
	Spellers::iterator it = spellers_.find(spellerID(lang, variety));
	if (it != spellers_.end())
		return to_aspell_speller(it->second.e_speller);
	
	return addSpeller(lang, variety);
}


string const AspellChecker::Private::spellerID(string const & lang,
					string const & variety)
{
	if (variety.empty())
		return lang;
	return lang + "-" + variety;
}


AspellChecker::AspellChecker(): d(new Private)
{
}


AspellChecker::~AspellChecker()
{
	delete d;
}


SpellChecker::Result AspellChecker::check(WordLangTuple const & word)
{
  
	AspellSpeller * m =
		d->speller(word.lang()->code(), word.lang()->variety());

	if (!m)
		return OK;

	if (word.word().empty())
		// MSVC compiled Aspell doesn't like it.
		return OK;

	int const word_ok = aspell_speller_check(m, to_utf8(word.word()).c_str(), -1);
	LASSERT(word_ok != -1, /**/);

	return (word_ok) ? OK : UNKNOWN_WORD;
}


void AspellChecker::insert(WordLangTuple const & word)
{
	Spellers::iterator it = d->spellers_.find(
		d->spellerID(word.lang()->code(), word.lang()->variety()));
	if (it != d->spellers_.end()) {
		AspellSpeller * speller = to_aspell_speller(it->second.e_speller);
		aspell_speller_add_to_personal(speller, to_utf8(word.word()).c_str(), -1);
	}
}


void AspellChecker::accept(WordLangTuple const & word)
{
	Spellers::iterator it = d->spellers_.find(
		d->spellerID(word.lang()->code(), word.lang()->variety()));
	if (it != d->spellers_.end()) {
		AspellSpeller * speller = to_aspell_speller(it->second.e_speller);
		aspell_speller_add_to_session(speller, to_utf8(word.word()).c_str(), -1);
	}
}


void AspellChecker::suggest(WordLangTuple const & wl,
	docstring_list & suggestions)
{
	suggestions.clear();
	AspellSpeller * m =
		d->speller(wl.lang()->code(), wl.lang()->variety());

	if (!m)
		return;

	AspellWordList const * sugs =
		aspell_speller_suggest(m, to_utf8(wl.word()).c_str(), -1);
	LASSERT(sugs != 0, /**/);
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


bool AspellChecker::hasDictionary(Language const * lang) const
{
	bool have = false;
	Spellers::iterator it = d->spellers_.begin();
	Spellers::iterator end = d->spellers_.end();

	if (lang) {
		for (; it != end && !have; ++it) {
			have = isValidDictionary(it->second.config, lang->code(), lang->variety());
		}
		if (!have) {
			AspellConfig * config = getConfig(lang->code(), lang->variety());
			have = isValidDictionary(config, lang->code(), lang->variety());
			delete_aspell_config(config);
		}
	}
	return have;
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
