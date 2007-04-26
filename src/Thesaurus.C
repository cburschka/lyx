/**
 * \file Thesaurus.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "Thesaurus.h"

#include "support/lstrings.h"

#include <algorithm>


namespace lyx {

#ifdef HAVE_LIBAIKSAURUS

using std::sort;
using std::string;


Thesaurus::Thesaurus()
	: aik_(new Aiksaurus)
{}


Thesaurus::~Thesaurus()
{
	delete aik_;
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
	if (!aik_->find(text.c_str()))
		return meanings;

	// weird api, but ...

	int prev_meaning = -1;
	int cur_meaning;
	docstring meaning;

	// correct, returns "" at the end
	string ret = aik_->next(cur_meaning);

	while (!ret.empty()) {
		if (cur_meaning != prev_meaning) {
			meaning = from_ascii(ret);
			ret = aik_->next(cur_meaning);
			prev_meaning = cur_meaning;
		} else {
			if (ret != text)
				meanings[meaning].push_back(from_ascii(ret));
		}

		ret = aik_->next(cur_meaning);
	}

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

#endif // HAVE_LIBAIKSAURUS

// Global instance
Thesaurus thesaurus;


} // namespace lyx
