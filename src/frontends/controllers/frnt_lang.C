/**
 * \file frnt_lang.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Angus Leeming
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>


#include "frnt_lang.h"
#include "gettext.h"
#include "language.h"

#include <algorithm>


using std::string;
using std::vector;


namespace lyx {
namespace frontend {

namespace {

class Sorter
	: public std::binary_function<LanguagePair,
				      LanguagePair, bool>
{
public:
	bool operator()(LanguagePair const & lhs,
			LanguagePair const & rhs) const {
		return lhs.first < rhs.first;
	}
};

} // namespace anon


vector<LanguagePair> const getLanguageData(bool character_dlg)
{
	vector<LanguagePair>::size_type const size = character_dlg ?
		languages.size() + 2 : languages.size();

	vector<LanguagePair> langs(size);

	if (character_dlg) {
		langs[0].first = _("No change");
		langs[0].second = "ignore";
		langs[1].first = _("Reset");
		langs[1].second = "reset";
	}

	vector<string>::size_type i = character_dlg ? 2 : 0;
	for (Languages::const_iterator cit = languages.begin();
	     cit != languages.end(); ++cit) {
		langs[i].first  = _(cit->second.display());
		langs[i].second = cit->second.lang();
		++i;
	}

	// Don't sort "ignore" and "reset"
	vector<LanguagePair>::iterator begin = character_dlg ?
		langs.begin() + 2 : langs.begin();

	std::sort(begin, langs.end(), Sorter());

	return langs;
}

} // namespace frontend
} // namespace lyx
