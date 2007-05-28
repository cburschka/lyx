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

#include "debug.h"
#include "support/lstrings.h"
#include "support/unicode.h"

#include <algorithm>


namespace lyx {

using std::sort;
using std::string;
using std::endl;


#ifndef HAVE_LIBMYTHES
#ifdef HAVE_LIBAIKSAURUS
Thesaurus::Thesaurus()
	: thes_(new Aiksaurus)
{}


Thesaurus::~Thesaurus()
{
	delete thes_;
}


Thesaurus::Meanings Thesaurus::lookup(docstring const & t)
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
{
	string const idx("/home/juergen/updates/MyThes-1.0/th_de_DE_v2.idx");
	string const data("/home/juergen/updates/MyThes-1.0/th_de_DE_v2.dat");
	char const * af = idx.c_str();
	char const * df = data.c_str();
	thes_ = new MyThes(af, df);
}


Thesaurus::~Thesaurus()
{
	if (thes_)
		delete thes_;
}


Thesaurus::Meanings Thesaurus::lookup(docstring const & t)
{
	Meanings meanings;

	string const encoding = thes_->get_th_encoding();
	
	mentry * pmean;
	string const text = to_iconv_encoding(support::lowercase(t), encoding);
	int len = strlen(text.c_str());
	int count = thes_->Lookup(text.c_str(), len, &pmean);
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
        thes_->CleanUpAfterLookup(&pmean, count);

	for (Meanings::iterator it = meanings.begin();
	     it != meanings.end(); ++it)
		sort(it->second.begin(), it->second.end());

	return meanings;
}

#else

Thesaurus::Thesaurus()
{
}


Thesaurus::~Thesaurus()
{
}


Thesaurus::Meanings Thesaurus::lookup(docstring const &)
{
	return Meanings();
}

#endif // HAVE_LIBMYTHES

// Global instance
Thesaurus thesaurus;


} // namespace lyx
