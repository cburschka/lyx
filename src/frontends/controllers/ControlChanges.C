/**
 * \file ControlChanges.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
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

using std::string;

namespace lyx {

namespace frontend {


ControlChanges::ControlChanges(Dialog & parent)
	: Dialog::Controller(parent)
{}


bool ControlChanges::find()
{
	return findNextChange(kernel().bufferview());
}


bool ControlChanges::changed()
{
	Change c(kernel().bufferview()->getCurrentChange());
	return c.type != Change::UNCHANGED;
}


docstring const ControlChanges::getChangeDate()
{
	Change c(kernel().bufferview()->getCurrentChange());
	if (c.type == Change::UNCHANGED || !c.changetime)
		return docstring();

	// FIXME UNICODE
	return from_utf8(formatted_time(c.changetime));
}


docstring const ControlChanges::getChangeAuthor()
{
	Change c(kernel().bufferview()->getCurrentChange());
	if (c.type == Change::UNCHANGED)
		return docstring();

	Author const & a(kernel().buffer().params().authors().get(c.author));

	// FIXME UNICODE in Author class
	docstring author(from_utf8(a.name()));

	if (!a.email().empty()) {
		author += " (";
		author += from_utf8(a.email());
		author += ")";
	}

	return author;
}


bool ControlChanges::accept()
{
	kernel().dispatch(FuncRequest(LFUN_CHANGE_ACCEPT));
	return findNextChange(kernel().bufferview());
}


bool ControlChanges::reject()
{
	kernel().dispatch(FuncRequest(LFUN_CHANGE_REJECT));
	return findNextChange(kernel().bufferview());
}


} // namespace frontend
} // namespace lyx
