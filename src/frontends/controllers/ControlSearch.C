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

using std::string;

namespace lyx {
namespace frontend {

ControlSearch::ControlSearch(Dialog & parent)
	: Dialog::Controller(parent)
{}


void ControlSearch::find(string const & search, bool casesensitive,
			 bool matchword, bool forward)
{
	string const data = find::find2string(search, casesensitive,
					      matchword, forward);
	kernel().dispatch(FuncRequest(LFUN_WORD_FIND, data));
}


void ControlSearch::replace(string const & search, string const & replace,
			    bool casesensitive, bool matchword,
			    bool forward, bool all)
{
	string const data =
		find::replace2string(search, replace, casesensitive,
				     matchword, all, forward);
	kernel().dispatch(FuncRequest(LFUN_WORD_REPLACE, data));
}

} // namespace frontend
} // namespace lyx
