// -*- C++ -*-
/* This file is part of
 * ====================================================== 
 *
 *           LyX, The Document Processor
 *
 *           Copyright 2001 The LyX Team.
 *
 * ======================================================
 *
 * \file ControlToc.C
 * \author Angus Leeming <a.leeming@ic.ac.uk>
 */

#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include "ViewBase.h"
#include "ButtonControllerBase.h"
#include "ControlToc.h"
#include "buffer.h"
#include "Dialogs.h"
#include "LyXView.h"
#include "lyxfunc.h"
#include "support/lstrings.h" // tostr
#include "gettext.h"

using std::vector;
using SigC::slot;

ControlToc::ControlToc(LyXView & lv, Dialogs & d)
	: ControlCommand(lv, d, LFUN_TOC_INSERT)
{
	d_.showTOC.connect(slot(this, &ControlToc::showInset));
	d_.createTOC.connect(slot(this, &ControlToc::createInset));
}


void ControlToc::Goto(int const & id) const
{
	string const tmp = tostr(id);
	lv_.getLyXFunc()->Dispatch(LFUN_GOTO_PARAGRAPH, tmp);
}


vector<string> const ControlToc::getTypes() const
{
	vector<string> types;

	Buffer::Lists const tmp = lv_.view()->buffer()->getLists();

	Buffer::Lists::const_iterator cit = tmp.begin();
	Buffer::Lists::const_iterator end = tmp.end();

	for (; cit != end; ++cit) {
		types.push_back(cit->first);
	}

	return types;
}


Buffer::SingleList const ControlToc::getContents(string const & type) const
{
	Buffer::SingleList empty_list;

	// This shouldn't be possible...
	if (!lv_.view()->available()) {
		return empty_list;
	}

	Buffer::Lists tmp = lv_.view()->buffer()->getLists();

	Buffer::Lists::iterator it = tmp.find(type);

	if (it == tmp.end()) {
		return empty_list;
	}

	return it->second;
}


namespace toc 
{

string const getType(string const & cmdName)
{
	string type;

	// It would be nice to have a map to extract this info.
	// Does one already exist, Lars?
	if (cmdName == "tableofcontents" )
		type = "TOC";

	return cmdName;
}
 
} // namespace toc 
