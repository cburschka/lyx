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

#include "ControlToc.h"
#include "toc.h"
#include "Dialogs.h"
#include "BufferView.h"

#include "support/lstrings.h" // tostr

using std::vector;

class Buffer;

ControlToc::ControlToc(LyXView & lv, Dialogs & d)
	: ControlCommand(lv, d, LFUN_TOC_INSERT)
{}


void ControlToc::goTo(toc::TocItem const & item) const
{
	item.goTo(lv_);
}


vector<string> const ControlToc::getTypes() const
{
	return toc::getTypes(lv_.view()->buffer());
}


toc::Toc const ControlToc::getContents(string const & type) const
{
	toc::Toc empty_list;

	// This shouldn't be possible...
	if (!lv_.view()->available()) {
		return empty_list;
	}

	toc::TocList tmp = toc::getTocList(lv_.view()->buffer());
	toc::TocList::iterator it = tmp.find(type);
	if (it == tmp.end()) {
		return empty_list;
	}

	return it->second;
}
