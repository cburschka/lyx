/*
 * \file frnt_lang.C
 * Copyright 2002 the LyX Team
 * Read the file COPYING
 *
 * \author Angus Leeming, a.leeming@ic.ac.uk
 */

#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include "frnt_lang.h"
#include "gettext.h"
#include "language.h"
#include <iostream>
#include <algorithm>

using std::vector;

namespace {

struct Sorter {
	bool operator()(frnt::LanguagePair const & lhs,
			frnt::LanguagePair const & rhs) const
        {
                return lhs.first < rhs.first;
        }
};

} // namespace anon

namespace frnt {

vector<LanguagePair> const getLanguageData(bool character_dlg)
{
	vector<LanguagePair>::size_type const size = character_dlg ?
		languages.size() + 2 : languages.size();

	vector<LanguagePair> langs(size);

	if (character_dlg) {
		langs[0].first = N_("No change"); langs[0].second = "No change";
		langs[1].first = N_("Reset");     langs[1].second = "Reset";
	}

	vector<string>::size_type i = character_dlg ? 2 : 0;
	for (Languages::const_iterator cit = languages.begin();
	     cit != languages.end(); ++cit) {
		langs[i].first  = _(cit->second.display());
		langs[i].second = cit->second.lang();
		++i;
	}

	std::sort(langs.begin(), langs.end(), Sorter());

	return langs;
}

} // namespace frnt
