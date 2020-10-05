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
                     TextEntry start_, TextEntry end_, Buffer const * buffer_)
	: error(error_), description(description_), start(start_), end(end_),
	  buffer(buffer_)
{}


ErrorItem::ErrorItem(docstring const & error_, docstring const & description_,
                     Buffer const * buffer_)
	: error(error_), description(description_), start(TexRow::text_none),
	  end(TexRow::text_none), buffer(buffer_)
{}


ErrorItem::ErrorItem()
	: start(TexRow::text_none), end(TexRow::text_none), buffer(nullptr)
{}


} // namespace lyx
