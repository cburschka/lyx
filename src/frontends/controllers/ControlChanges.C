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
#include "support/lstrings.h"

using lyx::support::rtrim;


ControlChanges::ControlChanges(Dialog & parent)
	: Dialog::Controller(parent)
{}


bool ControlChanges::find()
{
	return lyx::find::findNextChange(kernel().bufferview());
}


string const ControlChanges::getChangeDate()
{
	Change c(kernel().bufferview()->getCurrentChange());
	if (c.type == Change::UNCHANGED || !c.changetime)
		return string();

	// ctime adds newline; trim it off!
	string const date = rtrim(ctime(&c.changetime), "\n");
	return date;
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


void ControlChanges::accept()
{
	kernel().dispatch(FuncRequest(LFUN_ACCEPT_CHANGE));
	lyx::find::findNextChange(kernel().bufferview());
}


void ControlChanges::reject()
{
	kernel().dispatch(FuncRequest(LFUN_REJECT_CHANGE));
	lyx::find::findNextChange(kernel().bufferview());
}
