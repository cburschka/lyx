/**
 * \file ControlChanges.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 * \author Michael Gerz
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "ControlChanges.h"

#include "Author.h"
#include "Buffer.h"
#include "BufferParams.h"
#include "BufferView.h"
#include "Changes.h"
#include "FuncRequest.h"
#include "lyxfind.h"
#include "LyXRC.h"

#include "support/lyxtime.h"


namespace lyx {

namespace frontend {


ControlChanges::ControlChanges(Dialog & parent)
	: Controller(parent)
{}


void ControlChanges::next()
{
	dispatch(FuncRequest(LFUN_CHANGE_NEXT));
}


docstring const ControlChanges::getChangeDate()
{
	Change const & c = bufferview()->getCurrentChange();
	if (c.type == Change::UNCHANGED)
		return docstring();

	// FIXME UNICODE
	return from_utf8(formatted_time(c.changetime, lyxrc.date_insert_format));
}


docstring const ControlChanges::getChangeAuthor()
{
	Change const & c = bufferview()->getCurrentChange();
	if (c.type == Change::UNCHANGED)
		return docstring();

	Author const & a = buffer().params().authors().get(c.author);

	docstring author = a.name();

	if (!a.email().empty())
		author += " (" + a.email() + ")";

	return author;
}


void ControlChanges::accept()
{
	dispatch(FuncRequest(LFUN_CHANGE_ACCEPT));
	next();
}


void ControlChanges::reject()
{
	dispatch(FuncRequest(LFUN_CHANGE_REJECT));
	next();
}


} // namespace frontend
} // namespace lyx
