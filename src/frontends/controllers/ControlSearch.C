/**
 * \file ControlSearch.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Angus Leeming
 *
 * Full author contact details are available in file CREDITS
 */

#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include "ControlSearch.h"

#include "gettext.h"
#include "lyxfind.h"

#include "frontends/LyXView.h"

#include "support/lstrings.h"


ControlSearch::ControlSearch(LyXView & lv, Dialogs & d)
	: ControlDialogBD(lv, d)
{}


void ControlSearch::find(string const & search,
			 bool casesensitive, bool matchword, bool forward)
{
	bool const found = lyxfind::LyXFind(bufferview(), search,
					    forward, casesensitive,
					    matchword);

	if (!found)
		lv_.message(_("String not found!"));
}


void ControlSearch::replace(string const & search, string const & replace,
			    bool casesensitive, bool matchword, bool all)
{
	// If not replacing all instances of the word, then do not
	// move on to the next instance once the present instance has been
	// changed
	bool const once = !all;
	int const replace_count =
		lyxfind::LyXReplace(bufferview(),
				    search, replace, true, casesensitive,
				    matchword, all, once);

	if (replace_count == 0) {
		lv_.message(_("String not found!"));
	} else {
		if (replace_count == 1) {
			lv_.message(_("String has been replaced."));
		} else {
			string str = tostr(replace_count);
			str += _(" strings have been replaced.");
			lv_.message(str);
		}
	}
}
