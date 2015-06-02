/**
 * \file Thesaurus.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 * \author Jürgen Spitzmüller
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "Thesaurus.h"

#include "LyXRC.h"

#include "SpellChecker.h"
#include "WordLangTuple.h"

#include "support/FileNameList.h"
#include "support/Package.h"
#include "support/debug.h"
#include "support/docstring_list.h"
#include "support/filetools.h"
#include "support/gettext.h"
#include "support/lstrings.h"
#include "support/os.h"

#ifdef USE_EXTERNAL_MYTHES
#include MYTHES_H_LOCATION
#else
#include <cstdio>
#include "support/mythes/mythes.hxx"
#endif

#include "frontends/alert.h"

#include <algorithm>
#include <cstring>
#include <fstream>

using namespace std;
using namespace lyx::support;
using namespace lyx::support::os;

namespace lyx {

namespace {

typedef std::map<docstring, MyThes *> Thesauri;

} // namespace anon

struct Thesaurus::Private
{
	~Private()
	{
		for (Thesauri::iterator it = thes_.begin();
		     it != thes_.end(); ++it) {
			delete it->second;
		}
	}
	///
	bool thesaurusAvailable(docstring const & lang) const
	{
		for (Thesauri::const_iterator it = thes_.begin();
			it != thes_.end(); ++it) {
				if (it->first == lang)
					if (it->second)
						return true;
		}
		return false;
	}

	///
	typedef std::pair<std::string, std::string> ThesFiles;
	///
	ThesFiles getThesaurus(string const & path, docstring const & lang);
	ThesFiles getThesaurus(docstring const & lang);
	/// add a thesaurus to the list
	bool addThesaurus(docstring const & lang);

	/// the thesauri
	Thesauri thes_;

	/// the location below system/user directory
	/// there the data+idx files lookup will happen
	const string dataDirectory(void) { return "thes"; }

};


pair<string,string> Thesaurus::Private::getThesaurus(string const & path, docstring const & lang)
{
	FileName base(path);
	if (!base.isDirectory()) {
		return make_pair(string(), string());
	}
	FileNameList const idx_files = base.dirList("idx");
	FileNameList const data_files = base.dirList("dat");
	string idx;
	string data;
	string basename;

	LYXERR(Debug::FILES, "thesaurus path: " << path);
	for (FileNameList::const_iterator it = idx_files.begin(); it != idx_files.end(); ++it) {
		basename = it->onlyFileNameWithoutExt();
		if (contains(basename, to_ascii(lang))) {
			// do not use more specific dicts.
			if (contains(basename, '_') && !contains(lang, '_'))
				continue;
			if (contains(basename, '-') && !contains(lang, '-'))
				continue;
			ifstream ifs(it->absFileName().c_str());
			if (ifs) {
				// check for appropriate version of index file
				string encoding; // first line is encoding
				int items = 0;   // second line is no. of items
				getline(ifs,encoding);
				ifs >> items;
				if (ifs.fail()) {
					LYXERR(Debug::FILES, "ignore irregular thesaurus idx file: " << it->absFileName());
					continue;
				}
				if (encoding.length() == 0 || encoding.find_first_of(',') != string::npos) {
					LYXERR(Debug::FILES, "ignore version1 thesaurus idx file: " << it->absFileName());
					continue;
				}
			}
			idx = it->absFileName();
			LYXERR(Debug::FILES, "selected thesaurus idx file: " << idx);
			break;
		}
	}
	if (idx.empty()) {
		// try with a more general dictionary
		docstring shortcode;
		if (contains(lang, '_')) {
			split(lang, shortcode, '_');
			LYXERR(Debug::FILES, "Did not find thesaurus for LANG code "
			       << lang << ". Trying with " << shortcode);
			return getThesaurus(path, shortcode);
		}
		else if (contains(lang, '-')) {
			split(lang, shortcode, '-');
			LYXERR(Debug::FILES, "Did not find thesaurus for LANG code "
			       << lang << ". Trying with " << shortcode);
			return getThesaurus(path, shortcode);
		}
		return make_pair(string(), string());
	}
	for (support::FileNameList::const_iterator it = data_files.begin(); it != data_files.end(); ++it) {
		if (contains(it->onlyFileName(), basename)) {
			data = it->absFileName();
			LYXERR(Debug::FILES, "selected thesaurus data file: " << data);
			break;
		}
	}
	return make_pair(idx, data);
}


pair<string,string> Thesaurus::Private::getThesaurus(docstring const & lang)
{
	string const thes_path = external_path(lyxrc.thesaurusdir_path);
	pair<string,string> result ;

	if (thesaurusAvailable(lang))
		return make_pair(string(), string());

	if (!thes_path.empty())
		result = getThesaurus(thes_path, lang);
	if (result.first.empty() || result.second.empty()) {
		string const sys_path = external_path(addName(lyx::support::package().system_support().absFileName(),dataDirectory())) ;
		result = getThesaurus(sys_path, lang);
	}
	if (result.first.empty() || result.second.empty()) {
		string const user_path = external_path(addName(lyx::support::package().user_support().absFileName(),dataDirectory())) ;
		result = getThesaurus(user_path, lang);
	}
	return result;
}


bool Thesaurus::Private::addThesaurus(docstring const & lang)
{
	if (thesaurusAvailable(lang))
		return true;

	ThesFiles files = getThesaurus(lang);
	string const idx = files.first;
	string const data = files.second;

	if (idx.empty() || data.empty())
		return false;

	char const * af = idx.c_str();
	char const * df = data.c_str();
	thes_[lang] = new MyThes(af, df);
	return true;
}


bool Thesaurus::thesaurusAvailable(docstring const & lang) const
{
	return d->thesaurusAvailable(lang);
}


bool Thesaurus::thesaurusInstalled(docstring const & lang) const
{
	if (thesaurusAvailable(lang))
		return true;
	pair<string, string> files = d->getThesaurus(lang);
	return (!files.first.empty() && !files.second.empty());
}


Thesaurus::Meanings Thesaurus::lookup(WordLangTuple const & wl)
{
	Meanings meanings;
	MyThes * mythes = 0;

	docstring const lang_code = from_ascii(wl.lang()->code());
	docstring const t = wl.word();

	if (!d->addThesaurus(lang_code))
		return meanings;

	for (Thesauri::const_iterator it = d->thes_.begin();
	     it != d->thes_.end(); ++it) {
		if (it->first == lang_code) {
			mythes = it->second;
			break;
		}
	}

	if (!mythes)
		return meanings;

	string const encoding = mythes->get_th_encoding();
	
	mentry * pmean;
	string const text = to_iconv_encoding(support::lowercase(t), encoding);
	int len = strlen(text.c_str());
	int count = mythes->Lookup(text.c_str(), len, &pmean);
	if (!count) {
		SpellChecker * speller = theSpellChecker();
		if (!speller)
			return meanings;
		docstring_list suggestions;
		speller->stem(wl, suggestions);
		for (size_t i = 0; i != suggestions.size(); ++i) {
			string const wordform = to_iconv_encoding(support::lowercase(suggestions[i]), encoding);
			len = strlen(wordform.c_str());
			count = mythes->Lookup(wordform.c_str(), len, &pmean);
			if (count)
				break;
		}
		if (!count)
			return meanings;
	}

	// don't change value of pmean or count
	// they are needed for the CleanUpAfterLookup routine
	mentry * pm = pmean;
	docstring meaning;
	for (int i = 0; i < count; i++) {
		vector<docstring> ret;
		meaning = from_iconv_encoding(string(pm->defn), encoding);
		// remove silly item
		if (support::prefixIs(meaning, '-'))
			meaning = support::ltrim(meaning, "- ");
		for (int j = 0; j < pm->count; j++) {
			ret.push_back(from_iconv_encoding(string(pm->psyns[j]), encoding));
		}
		meanings[meaning] = ret;
		++pm;
	}
	// now clean up all allocated memory
	mythes->CleanUpAfterLookup(&pmean, count);

	for (Meanings::iterator it = meanings.begin();
	     it != meanings.end(); ++it)
		sort(it->second.begin(), it->second.end());

	return meanings;
}


Thesaurus::Thesaurus()
	: d(new Thesaurus::Private)
{
}


Thesaurus::~Thesaurus()
{
	delete d;
}

// Global instance
Thesaurus thesaurus;


} // namespace lyx
