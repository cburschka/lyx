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

#include "funcrequest.h"
#include "lyxfind.h"

#include "frontends/LyXView.h"

using std::string;


/* The ControlSeach class is now in a fit state to derive from
   Dialog::Controller
*/
ControlSearch::ControlSearch(LyXView & lv, Dialogs & d)
	: ControlDialogBD(lv, d)
{}


void ControlSearch::find(string const & search,
			 bool casesensitive, bool matchword, bool forward)
{
	string const data =
		lyx::find::find2string(search,
				       casesensitive, matchword, forward);
	lv_.dispatch(FuncRequest(LFUN_WORD_FIND, data));
}


void ControlSearch::replace(string const & search, string const & replace,
			    bool casesensitive, bool matchword, bool forward, bool all)
{
	string const data =
		lyx::find::replace2string(search, replace,
					  casesensitive, matchword, all, forward);
	lv_.dispatch(FuncRequest(LFUN_WORD_REPLACE, data));
}
