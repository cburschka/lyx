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

#include "support/debug.h"
#include "support/gettext.h"
#include "LyXRC.h"

#include "support/FileNameList.h"
#include "support/filetools.h"
#include "support/lstrings.h"
#include "support/os.h"
#include "support/unicode.h"

#include "frontends/alert.h"

#include <algorithm>
#include <cstring>

using namespace std;
using namespace lyx::support;
using namespace lyx::support::os;

namespace lyx {

#ifndef HAVE_LIBMYTHES
#ifdef HAVE_LIBAIKSAURUS


Thesaurus::Thesaurus()
	: thes_(new Aiksaurus)
{}


Thesaurus::~Thesaurus()
{
	delete thes_;
}


Thesaurus::Meanings Thesaurus::lookup(docstring const & t, docstring const &)
{
	Meanings meanings;

	// aiksaurus is for english text only, therefore it does not work
	// with non-ascii strings.
	// The interface of the Thesaurus class uses docstring because a
	// non-english thesaurus is possible in theory.
	if (!support::isAscii(t))
		// to_ascii() would assert
		return meanings;

	string const text = to_ascii(t);

	docstring error = from_ascii(thes_->error());
	if (!error.empty()) {
		static bool sent_error = false;
		if (!sent_error) {
			frontend::Alert::error(_("Thesaurus failure"),
				     bformat(_("Aiksaurus returned the following error:\n\n%1$s."),
					     error));
			sent_error = true;
		}
		return meanings;
	}
	if (!thes_->find(text.c_str()))
		return meanings;

	// weird api, but ...

	int prev_meaning = -1;
	int cur_meaning;
	docstring meaning;

	// correct, returns "" at the end
	string ret = thes_->next(cur_meaning);

	while (!ret.empty()) {
		if (cur_meaning != prev_meaning) {
			meaning = from_ascii(ret);
			ret = thes_->next(cur_meaning);
			prev_meaning = cur_meaning;
		} else {
			if (ret != text)
				meanings[meaning].push_back(from_ascii(ret));
		}

		ret = thes_->next(cur_meaning);
	}

	for (Meanings::iterator it = meanings.begin();
	     it != meanings.end(); ++it)
		sort(it->second.begin(), it->second.end());

	return meanings;
}


bool Thesaurus::thesaurusAvailable(docstring const & lang) const
{
	// we support English only
	return prefixIs(lang, from_ascii("en_"));
}

#endif // HAVE_LIBAIKSAURUS
#endif // !HAVE_LIBMYTHES


#ifdef HAVE_LIBMYTHES

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

} // namespace anon


Thesaurus::Thesaurus()
{}


Thesaurus::~Thesaurus()
{
	for (Thesauri::iterator it = thes_.begin();
	     it != thes_.end(); ++it) {
		delete it->second;
	}
}


bool Thesaurus::addThesaurus(docstring const & lang)
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
	for (Thesauri::const_iterator it = thes_.begin();
	     it != thes_.end(); ++it) {
		if (it->first == lang)
			if (it->second)
				return true;
	}

	return false;
}


Thesaurus::Meanings Thesaurus::lookup(docstring const & t, docstring const & lang)
{
	Meanings meanings;
	MyThes * mythes = 0;

	if (!addThesaurus(lang))
		return meanings;

	for (Thesauri::const_iterator it = thes_.begin();
	     it != thes_.end(); ++it) {
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

#else
#ifndef HAVE_LIBAIKSAURUS
Thesaurus::Thesaurus()
{
}


Thesaurus::~Thesaurus()
{
}


Thesaurus::Meanings Thesaurus::lookup(docstring const &, docstring const &)
{
	return Meanings();
}


bool Thesaurus::thesaurusAvailable(docstring const & lang) const
{
	return false;
}

#endif
#endif // HAVE_LIBMYTHES

// Global instance
Thesaurus thesaurus;


} // namespace lyx
