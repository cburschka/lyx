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

#include "ViewBase.h"
#include "ButtonControllerBase.h"
#include "ControlChanges.h"
#include "frontends/Dialogs.h"
#include "frontends/LyXView.h"
#include "buffer.h"
#include "lyxfind.h"
#include "lyxfunc.h"
#include "debug.h"
#include "BufferView.h"
#include "support/lstrings.h"
#include "funcrequest.h"
#include "author.h"

ControlChanges::ControlChanges(LyXView & lv, Dialogs & d)
	: ControlDialogBD(lv, d)
{
}


void ControlChanges::find()
{
	lyxfind::findNextChange(bufferview());
}


string const ControlChanges::getChangeDate()
{
	Change c(bufferview()->getCurrentChange());
	if (c.type == Change::UNCHANGED || !c.changetime)
		return string();
	return ctime(&c.changetime);
}


string const ControlChanges::getChangeAuthor()
{
	Change c(bufferview()->getCurrentChange());
	if (c.type == Change::UNCHANGED)
		return string();

	Author const & a(bufferview()->buffer()->authors().get(c.author));

	string author(a.name());

	if (!a.email().empty()) {
		author += " (";
		author += a.email() + ")";
	}

	return author;
}


void ControlChanges::accept()
{
	lv_.dispatch(FuncRequest(LFUN_ACCEPT_CHANGE));
	lyxfind::findNextChange(bufferview());
}


void ControlChanges::reject()
{
	lv_.dispatch(FuncRequest(LFUN_REJECT_CHANGE));
	lyxfind::findNextChange(bufferview());
}
