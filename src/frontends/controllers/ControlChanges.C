/**
 * \file ControlChanges.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS
 */

#include <config.h>

#include "ControlChanges.h"
#include "frontends/Dialogs.h"
#include "frontends/LyXView.h"
#include "buffer.h"
#include "BufferView.h"
#include "funcrequest.h"
#include "lyxfind.h"
#include "author.h"
#include "support/lstrings.h"


ControlChanges::ControlChanges(Dialog & parent)
	: Dialog::Controller(parent)
{}


void ControlChanges::find()
{
	lyxfind::findNextChange(kernel().bufferview());
}


string const ControlChanges::getChangeDate()
{
	Change c(kernel().bufferview()->getCurrentChange());
	if (c.type == Change::UNCHANGED || !c.changetime)
		return string();
	return ctime(&c.changetime);
}


string const ControlChanges::getChangeAuthor()
{
	Change c(kernel().bufferview()->getCurrentChange());
	if (c.type == Change::UNCHANGED)
		return string();

	Author const & a(kernel().buffer()->authors().get(c.author));

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
	lyxfind::findNextChange(kernel().bufferview());
}


void ControlChanges::reject()
{
	kernel().dispatch(FuncRequest(LFUN_REJECT_CHANGE));
	lyxfind::findNextChange(kernel().bufferview());
}
