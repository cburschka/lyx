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
	return find::findNextChange(kernel().bufferview());
}


string const ControlChanges::getChangeDate()
{
	Change c(kernel().bufferview()->getCurrentChange());
	if (c.type == Change::UNCHANGED || !c.changetime)
		return string();

	return formatted_time(c.changetime);
}


string const ControlChanges::getChangeAuthor()
{
	Change c(kernel().bufferview()->getCurrentChange());
	if (c.type == Change::UNCHANGED)
		return string();

	Author const & a(kernel().buffer().params().authors().get(c.author));

	string author(a.name());

	if (!a.email().empty()) {
		author += " (";
		author += a.email() + ")";
	}

	return author;
}


bool ControlChanges::accept()
{
	kernel().dispatch(FuncRequest(LFUN_ACCEPT_CHANGE));
	return find::findNextChange(kernel().bufferview());
}


bool ControlChanges::reject()
{
	kernel().dispatch(FuncRequest(LFUN_REJECT_CHANGE));
	return find::findNextChange(kernel().bufferview());
}


} // namespace frontend
} // namespace lyx
