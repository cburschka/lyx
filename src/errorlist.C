/**
 * \file errorlist.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Alfredo Braunstein
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "errorlist.h"

using std::string;


ErrorItem::ErrorItem(string const & error, string const & description,
		     int par_id, int pos_start, int pos_end)
	: error(error), description(description), par_id(par_id),
	  pos_start(pos_start),  pos_end(pos_end)
{}


ErrorItem::ErrorItem()
	: par_id(-1),  pos_start(0),  pos_end(0)
{}
