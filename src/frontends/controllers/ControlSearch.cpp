/**
 * \file ControlSearch.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Angus Leeming
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "ControlSearch.h"

#include "FuncRequest.h"
#include "lyxfind.h"

namespace lyx {
namespace frontend {

ControlSearch::ControlSearch(Dialog & parent)
	: Controller(parent)
{}


void ControlSearch::find(docstring const & search, bool casesensitive,
			 bool matchword, bool forward)
{
	docstring const data = find2string(search, casesensitive,
					      matchword, forward);
	kernel().dispatch(FuncRequest(LFUN_WORD_FIND, data));
}


void ControlSearch::replace(docstring const & search, docstring const & replace,
			    bool casesensitive, bool matchword,
			    bool forward, bool all)
{
	docstring const data =
		replace2string(search, replace, casesensitive,
				     matchword, all, forward);
	kernel().dispatch(FuncRequest(LFUN_WORD_REPLACE, data));
}

} // namespace frontend
} // namespace lyx
