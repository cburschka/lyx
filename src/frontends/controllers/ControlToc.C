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

#include "ControlToc.h"
#include "support/lstrings.h" // tostr

using std::vector;

class Buffer;


ControlToc::ControlToc(Dialog & d)
	: ControlCommand(d, "toc")
{}


void ControlToc::goTo(lyx::toc::TocItem const & item)
{
	item.goTo(kernel().lyxview());
}


vector<string> const ControlToc::getTypes() const
{
	return lyx::toc::getTypes(kernel().buffer());
}


lyx::toc::Toc const ControlToc::getContents(string const & type) const
{
	lyx::toc::Toc empty_list;

	// This shouldn't be possible...
	if (!kernel().isBufferAvailable()) {
		return empty_list;
	}

	lyx::toc::TocList tmp = lyx::toc::getTocList(kernel().buffer());
	lyx::toc::TocList::iterator it = tmp.find(type);
	if (it == tmp.end()) {
		return empty_list;
	}

	return it->second;
}
