/* This file is part of
 * ======================================================
 *
 *           LyX, The Document Processor
 *
 *           Copyright 2001 The LyX Team.
 *
 * ======================================================
 *
 * \file ControlSearch.C
 * \author Angus Leeming <a.leeming@ic.ac.uk>
 */

#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include "ControlSearch.h"
#include "Liason.h"
#include "buffer.h"
#include "lyxfind.h"
#include "gettext.h"
#include "BufferView.h"

#include "frontends/LyXView.h"

#include "support/lstrings.h"

using Liason::setMinibuffer;


ControlSearch::ControlSearch(LyXView & lv, Dialogs & d)
	: ControlDialogBD(lv, d)
{}


void ControlSearch::find(string const & search,
			 bool casesensitive, bool matchword, bool forward) const
{
	bool const found = lyxfind::LyXFind(lv_.view().get(), search,
					    forward, casesensitive,
					    matchword);

	if (!found)
		setMinibuffer(&lv_, _("String not found!"));
}


void ControlSearch::replace(string const & search, string const & replace,
			    bool casesensitive, bool matchword, bool all) const
{
	// If not replacing all instances of the word, then do not
	// move on to the next instance once the present instance has been
	// changed
	bool const once = !all;
	int const replace_count =
		lyxfind::LyXReplace(lv_.view().get(),
				    search, replace, true, casesensitive,
				    matchword, all, once);

	if (replace_count == 0) {
		setMinibuffer(&lv_, _("String not found!"));
	} else {
		if (replace_count == 1) {
			setMinibuffer(&lv_, _("String has been replaced."));
		} else {
			string str = tostr(replace_count);
			str += _(" strings have been replaced.");
			setMinibuffer(&lv_, str.c_str());
		}
	}
}
