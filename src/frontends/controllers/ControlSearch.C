/**
 * \file ControlSearch.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Angus Leeming
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "ControlSearch.h"

#include "gettext.h"
#include "lyxfind.h"

#include "frontends/LyXView.h"

#include "support/tostr.h"


using std::string;


ControlSearch::ControlSearch(LyXView & lv, Dialogs & d)
	: ControlDialogBD(lv, d)
{}


void ControlSearch::find(string const & search,
			 bool casesensitive, bool matchword, bool forward)
{
	bool const found = lyx::find::find(bufferview(), search,
					   casesensitive, matchword,
					   forward);

	if (!found)
		lv_.message(_("String not found!"));
}


void ControlSearch::replace(string const & search, string const & replace,
			    bool casesensitive, bool matchword,
			    bool forward, bool all)
{
	// If not replacing all instances of the word, then do not
	// move on to the next instance once the present instance has been
	// changed
	int const replace_count = all ?
		lyx::find::replaceAll(bufferview(), search, replace,
				      casesensitive, matchword)
		: lyx::find::replace(bufferview(), search, replace,
				     casesensitive, matchword, forward);

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
