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
#include "Dialogs.h"
#include "Liason.h"
#include "LyXView.h"
#include "buffer.h"
#include "lyxfind.h"
#include "debug.h"

using Liason::setMinibuffer;
using SigC::slot;

ControlSearch::ControlSearch(LyXView & lv, Dialogs & d)
	: ControlDialog<ControlConnectBD>(lv, d)
{
	d_.showSearch.connect(SigC::slot(this, &ControlSearch::show));

	// perhaps in the future we'd like a
	// "search again" button/keybinding
	// d_.searchAgain.connect(SigC::slot(this, &ControlSearch::FindNext));
}


void ControlSearch::find(string const & search,
			 bool casesensitive, bool matchword, bool forward) const
{
	bool const found = LyXFind(lv_.view(), search, casesensitive,
				   matchword, forward);
   
	if (!found)
		setMinibuffer(&lv_, _("String not found!"));
}


void ControlSearch::replace(string const & search, string const & replace,
			    bool casesensitive, bool matchword, bool all) const
{
	int const replace_count = LyXReplace(lv_.view(),
					     search, replace, casesensitive, 
					     matchword, true, all);
				  
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
