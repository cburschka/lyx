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

#include "support/FileName.h"
#include "support/Path.h"

#include <aspell.h>

#include <map>
#include <string>

using namespace std;

namespace lyx {

namespace {

struct Speller {
	AspellSpeller * speller;
	AspellConfig * config;
};

typedef std::map<std::string, Speller> Spellers;

} // anon namespace

struct AspellChecker::Private
{
	Private(): spell_error_object(0) {}

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

	/// FIXME
	AspellCanHaveError * spell_error_object;
};


AspellChecker::Private::~Private()
{
	if (spell_error_object) {
		delete_aspell_can_have_error(spell_error_object);
		spell_error_object = 0;
	}

	Spellers::iterator it = spellers_.begin();
	Spellers::iterator end = spellers_.end();

	for (; it != end; ++it) {
		aspell_speller_save_all_word_lists(it->second.speller);
		delete_aspell_speller(it->second.speller);
		delete_aspell_config(it->second.config);
	}
}


AspellConfig * getConfig()
{
	AspellConfig * config = new_aspell_config();
#ifdef __APPLE__
	char buf[2048] ;
	bool have_dict = false;
#ifdef ASPELL_FRAMEWORK
	char * framework = ASPELL_FRAMEWORK ;

	if ( strlen(framework) && getPrivateFrameworkPathName(buf, sizeof(buf), framework) ) {
		lyx::support::FileName const base(buf);
		lyx::support::FileName const data(base.absFilename() + "/Resources/data");
		lyx::support::FileName const dict(base.absFilename() + "/Resources/dict");
		LYXERR(Debug::FILES, "aspell bundle path: " << buf);
		have_dict = dict.isDirectory() && data.isDirectory();
		if (have_dict) {
			aspell_config_replace(config, "dict-dir", dict.absFilename().c_str());
			aspell_config_replace(config, "data-dir", data.absFilename().c_str());
			LYXERR(Debug::FILES, "aspell dict: " << dict);
		}
	}
#endif
	if ( !have_dict ) {
		lyx::support::FileName const base("/opt/local"); // check for mac-ports data
		lyx::support::FileName const data(base.absFilename() + "/lib/aspell-0.60");
		lyx::support::FileName const dict(base.absFilename() + "/share/aspell");
		have_dict = dict.isDirectory() && data.isDirectory();
		if (have_dict) {
			aspell_config_replace(config, "dict-dir", dict.absFilename().c_str());
			aspell_config_replace(config, "data-dir", data.absFilename().c_str());
			LYXERR(Debug::FILES, "aspell dict: " << dict);
		}
	}
#endif
	return config ;
}


AspellSpeller * AspellChecker::Private::addSpeller(string const & lang,
						   string const & variety)
{
	AspellConfig * config = getConfig();
	// Aspell supports both languages and varieties (such as German
	// old vs. new spelling). The respective naming convention is
	// lang_REGION-variety (e.g. de_DE-alt).
	aspell_config_replace(config, "lang", lang.c_str());
	if (!variety.empty())
		aspell_config_replace(config, "variety", variety.c_str());
	// Set the encoding to utf-8.
	// aspell does also understand "ucs-4", so we would not need a
	// conversion in theory, but if this is used it expects all
	// char const * arguments to be a cast from  uint const *, and it
	// seems that this uint is not compatible with our char_type on some
	// platforms (cygwin, OS X). Therefore we use utf-8, that does
	// always work.
	aspell_config_replace(config, "encoding", "utf-8");
	if (lyxrc.spellchecker_accept_compound)
		// Consider run-together words as legal compounds
		aspell_config_replace(config, "run-together", "true");
	else
		// Report run-together words as errors
		aspell_config_replace(config, "run-together", "false");

	AspellCanHaveError * err = new_aspell_speller(config);
	if (spell_error_object)
		delete_aspell_can_have_error(spell_error_object);
	spell_error_object = 0;

	if (aspell_error_number(err) != 0) {
		// FIXME: We should we indicate somehow that this language is not
		// supported.
		spell_error_object = err;
		LYXERR(Debug::FILES, "aspell error: " << aspell_error_message(err));
		return 0;
	}
	Speller m;
	m.speller = to_aspell_speller(err);
	m.config = config;
	spellers_[spellerID(lang, variety)] = m;
	return m.speller;
}


AspellSpeller * AspellChecker::Private::speller(string const & lang,
						string const & variety)
{
	Spellers::iterator it = spellers_.find(spellerID(lang, variety));
	if (it != spellers_.end())
		return it->second.speller;
	
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
	if (it != d->spellers_.end())
		aspell_speller_add_to_personal(it->second.speller, to_utf8(word.word()).c_str(), -1);
}


void AspellChecker::accept(WordLangTuple const & word)
{
	Spellers::iterator it = d->spellers_.find(
		d->spellerID(word.lang()->code(), word.lang()->variety()));
	if (it != d->spellers_.end())
		aspell_speller_add_to_session(it->second.speller, to_utf8(word.word()).c_str(), -1);
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
	if (!lang)
		return false;
	// code taken from aspell's list-dicts example
	AspellConfig * config;
	AspellDictInfoList * dlist;
	AspellDictInfoEnumeration * dels;
	const AspellDictInfo * entry;

	config = getConfig();

	/* the returned pointer should _not_ need to be deleted */
	dlist = get_aspell_dict_info_list(config);

	/* config is no longer needed */
	delete_aspell_config(config);

	dels = aspell_dict_info_list_elements(dlist);

	bool have = false;
	while ((entry = aspell_dict_info_enumeration_next(dels)) != 0)
	{
		if (entry->code == lang->code()
		    && (lang->variety().empty() || entry->jargon == lang->variety())) {
			have = true;
			break;
		}
	}

	delete_aspell_dict_info_enumeration(dels);

	return have;
}


docstring const AspellChecker::error()
{
	char const * err = 0;

	if (d->spell_error_object && aspell_error_number(d->spell_error_object) != 0)
		err = aspell_error_message(d->spell_error_object);

	// FIXME UNICODE: err is not in UTF8, but probably the locale encoding
	return (err ? from_utf8(err) : docstring());
}


} // namespace lyx
