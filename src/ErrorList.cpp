/**
 * \file ErrorList.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Alfredo Braunstein
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "ErrorList.h"

using namespace std;

namespace lyx {


ErrorItem::ErrorItem(docstring const & error_, docstring const & description_,
		     int par_id_, pos_type pos_start_, pos_type pos_end_)
	: error(error_), description(description_), par_id(par_id_),
	  pos_start(pos_start_),  pos_end(pos_end_)
{}


ErrorItem::ErrorItem()
	: par_id(-1), pos_start(0), pos_end(0)
{}


} // namespace lyx
