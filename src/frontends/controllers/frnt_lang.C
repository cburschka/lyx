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

using std::vector;

namespace frnt {

vector<LanguagePair> const getLanguageData()
{
	vector<LanguagePair> langs(languages.size() + 2);

	langs[0].first = N_("No change"); langs[0].second = "No change";
	langs[1].first = N_("Reset");     langs[1].second = "Reset";

	vector<string>::size_type i = 2;
	for (Languages::const_iterator cit = languages.begin();
	     cit != languages.end(); ++cit) {
		langs[i].first  = cit->second.display();
		langs[i].second = cit->second.lang();
		++i;
	}

	return langs;
}

} // namespace frnt
