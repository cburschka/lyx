/**
 * \file Thesaurus.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "Thesaurus.h"

#include "gettext.h"

#include "support/lstrings.h"

#include "frontends/Alert.h"

#include <algorithm>
#include <string>

using std::string;

#ifdef HAVE_LIBAIKSAURUS
using lyx::support::bformat;

using std::sort;


Thesaurus::Thesaurus()
	: aik_(new Aiksaurus)
{}


Thesaurus::~Thesaurus()
{
	delete aik_;
}


Thesaurus::Meanings Thesaurus::lookup(string const & text)
{
	Meanings meanings;

	string error = string(aik_->error());
	if (!error.empty()) {
		static bool sent_error = false;
		if (!sent_error) {
			Alert::error(_("Thesaurus failure"),
				     bformat(_("Aiksaurus returned the following error:\n\n%1$s."),
					     error));
			sent_error = true;
		}
		return meanings;
	}
	if (!aik_->find(text.c_str()))
		return meanings;

	// weird api, but ...

	int prev_meaning = -1;
	int cur_meaning;
	string meaning;

	// correct, returns "" at the end
	string ret = aik_->next(cur_meaning);

	while (!ret.empty()) {
		if (cur_meaning != prev_meaning) {
			meaning = ret;
			ret = aik_->next(cur_meaning);
			prev_meaning = cur_meaning;
		} else {
			if (ret != text) {
				meanings[meaning].push_back(ret);
			}
		}

		ret = aik_->next(cur_meaning);
	}

	for (Meanings::iterator it = meanings.begin();
		it != meanings.end(); ++it) {
			sort(it->second.begin(), it->second.end());
	}

	return meanings;
}

#else

Thesaurus::Thesaurus()
{
}


Thesaurus::~Thesaurus()
{
}


Thesaurus::Meanings Thesaurus::lookup(string const &)
{
	return Meanings();
}

#endif // HAVE_LIBAIKSAURUS

// Global instance
Thesaurus thesaurus;
