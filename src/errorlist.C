/**
 * \file errorlist.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Alfredo Braunstein
 *
 * Full author contact details are available in file CREDITS
 */

#include <config.h>

#include "errorlist.h"
#include "buffer.h"
#include "LaTeX.h"


ErrorItem::ErrorItem(string const & error, string const & description,
		     int par_id, int pos_start, int pos_end)
	: error(error), description(description), par_id(par_id),
	  pos_start(pos_start),  pos_end(pos_end)
{}


ErrorItem::ErrorItem()
	: par_id(-1),  pos_start(0),  pos_end(0)
{}


ErrorList::ErrorList(Buffer const & buf, 
		     TeXErrors const & terr) 
{
	TeXErrors::Errors::const_iterator cit = terr.begin();
	TeXErrors::Errors::const_iterator end = terr.end();

	for (; cit != end; ++cit) {
		int par_id = -1;
		int posstart = -1;
		int const errorrow = cit->error_in_line;
		buf.texrow.getIdFromRow(errorrow, par_id, posstart);
		int posend = -1;
		buf.texrow.getIdFromRow(errorrow + 1, par_id, posend);
		push_back(ErrorItem(cit->error_desc,
				    cit->error_text,
				    par_id, posstart, posend));
	}
}
