/**
 * \file ControlToc.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Angus Leeming
 *
 * Full author contact details are available in file CREDITS
 */

#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include "ControlToc.h"
#include "toc.h"

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
	return toc::getTypes(buffer());
}


toc::Toc const ControlToc::getContents(string const & type) const
{
	toc::Toc empty_list;

	// This shouldn't be possible...
	if (!bufferIsAvailable()) {
		return empty_list;
	}

	toc::TocList tmp = toc::getTocList(buffer());
	toc::TocList::iterator it = tmp.find(type);
	if (it == tmp.end()) {
		return empty_list;
	}

	return it->second;
}
