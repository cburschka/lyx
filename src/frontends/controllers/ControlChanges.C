/**
 * \file ControlChanges.C
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

#include "author.h"
#include "buffer.h"
#include "bufferparams.h"
#include "BufferView.h"
#include "changes.h"
#include "funcrequest.h"
#include "lyxfind.h"

#include "support/lyxtime.h"


namespace lyx {

namespace frontend {


ControlChanges::ControlChanges(Dialog & parent)
	: Dialog::Controller(parent)
{}


void ControlChanges::next()
{
	kernel().dispatch(FuncRequest(LFUN_CHANGE_NEXT));
}


docstring const ControlChanges::getChangeDate()
{
	Change const & c = kernel().bufferview()->getCurrentChange();
	if (c.type == Change::UNCHANGED)
		return docstring();

	// FIXME UNICODE
	return from_utf8(formatted_time(c.changetime));
}


docstring const ControlChanges::getChangeAuthor()
{
	Change const & c = kernel().bufferview()->getCurrentChange();
	if (c.type == Change::UNCHANGED)
		return docstring();

	Author const & a = kernel().buffer().params().authors().get(c.author);

	docstring author(a.name());

	if (!a.email().empty()) {
		author += " (" + a.email() + ")";
	}

	return author;
}


void ControlChanges::accept()
{
	kernel().dispatch(FuncRequest(LFUN_CHANGE_ACCEPT));
	next();
}


void ControlChanges::reject()
{
	kernel().dispatch(FuncRequest(LFUN_CHANGE_REJECT));
	next();
}


} // namespace frontend
} // namespace lyx
