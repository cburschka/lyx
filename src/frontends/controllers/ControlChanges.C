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

// FIXME: those two headers are needed because of the
// WorkArea::redraw() call below.
#include "frontends/LyXView.h"
#include "frontends/WorkArea.h"

#include "support/lyxtime.h"

using std::string;

namespace lyx {

namespace frontend {


ControlChanges::ControlChanges(Dialog & parent)
	: Dialog::Controller(parent)
{}


bool ControlChanges::find()
{
	// FIXME: it would be better to use an LFUN.
	if (!findNextChange(kernel().bufferview()))
		return false;

	kernel().bufferview()->update();
	kernel().lyxview().currentWorkArea()->redraw();
	return true;
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

	docstring author(a.name());

	if (!a.email().empty()) {
		author += " (";
		author += a.email();
		author += ")";
	}

	return author;
}


bool ControlChanges::accept()
{
	kernel().dispatch(FuncRequest(LFUN_CHANGE_ACCEPT));
	return find();
}


bool ControlChanges::reject()
{
	kernel().dispatch(FuncRequest(LFUN_CHANGE_REJECT));
	return find();
}


} // namespace frontend
} // namespace lyx
