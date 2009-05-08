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

#include "support/FileNameList.h"
#include "support/debug.h"
#include "support/filetools.h"
#include "support/gettext.h"
#include "support/lstrings.h"
#include "support/os.h"
#include "support/unicode.h"

#include "support/mythes/mythes.hxx"

#include "frontends/alert.h"

#include <algorithm>
#include <cstring>

using namespace std;
using namespace lyx::support;
using namespace lyx::support::os;

namespace lyx {

namespace {

string const to_iconv_encoding(docstring const & s, string const & encoding)
{
	std::vector<char> const encoded =
		ucs4_to_eightbit(s.data(), s.length(), encoding);
	return string(encoded.begin(), encoded.end());
}


docstring const from_iconv_encoding(string const & s, string const & encoding)
{
	std::vector<char_type> const ucs4 =
		eightbit_to_ucs4(s.data(), s.length(), encoding);
	return docstring(ucs4.begin(), ucs4.end());
}

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

	/// add a thesaurus to the list
	bool addThesaurus(docstring const & lang);

	/// the thesauri
	Thesauri thes_;
};

bool Thesaurus::Private::addThesaurus(docstring const & lang)
{
	string const thes_path = external_path(lyxrc.thesaurusdir_path);
	LYXERR(Debug::FILES, "thesaurus path: " << thes_path);
	if (thes_path.empty())
		return false;

	if (thesaurusAvailable(lang))
		return true;

	FileNameList const idx_files = FileName(thes_path).dirList("idx");
	FileNameList const data_files = FileName(thes_path).dirList("dat");
	string idx;
	string data;

	for (FileNameList::const_iterator it = idx_files.begin();
	     it != idx_files.end(); ++it) {
		LYXERR(Debug::FILES, "found thesaurus idx file: " << it->onlyFileName());
		if (contains(it->onlyFileName(), to_ascii(lang))) {
			idx = it->absFilename();
			LYXERR(Debug::FILES, "selected thesaurus idx file: " << idx);
			break;
			}
		}

	for (support::FileNameList::const_iterator it = data_files.begin();
	     it != data_files.end(); ++it) {
		LYXERR(Debug::FILES, "found thesaurus data file: " << it->onlyFileName());
		if (contains(it->onlyFileName(), to_ascii(lang))) {
			data = it->absFilename();
			LYXERR(Debug::FILES, "selected thesaurus data file: " << data);
			break;
			}
		}

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


Thesaurus::Meanings Thesaurus::lookup(docstring const & t, docstring const & lang)
{
	Meanings meanings;
	MyThes * mythes = 0;

	if (!d->addThesaurus(lang))
		return meanings;

	for (Thesauri::const_iterator it = d->thes_.begin();
	     it != d->thes_.end(); ++it) {
		if (it->first == lang) {
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
	if (!count)
		return meanings;

	// don't change value of pmean or count
	// they are needed for the CleanUpAfterLookup routine
	mentry * pm = pmean;
	docstring meaning;
	docstring ret;
	for (int i = 0; i < count; i++) {
		meaning = from_iconv_encoding(string(pm->defn), encoding);
		// remove silly item
		if (support::prefixIs(meaning, '-'))
			meaning = support::ltrim(meaning, "- ");
		for (int j = 0; j < pm->count; j++) {
			ret = from_iconv_encoding(string(pm->psyns[j]), encoding);
		}
	meanings[meaning].push_back(ret);
	pm++;
	}
	// now clean up all allocated memory
	mythes->CleanUpAfterLookup(&pmean, count);

	for (Meanings::iterator it = meanings.begin();
	     it != meanings.end(); ++it)
		sort(it->second.begin(), it->second.end());

	return meanings;
}


Thesaurus::Thesaurus() : d(new Thesaurus::Private)
{
}


Thesaurus::~Thesaurus()
{
	delete d;
}

// Global instance
Thesaurus thesaurus;


} // namespace lyx
