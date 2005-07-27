/**
 * \file ControlToc.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Angus Leeming
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "ControlToc.h"
#include "gettext.h"

using std::vector;
using std::string;


class Buffer;

namespace lyx {
namespace frontend {


ControlToc::ControlToc(Dialog & d)
	: ControlCommand(d, "toc")
{}


void ControlToc::goTo(toc::TocItem const & item)
{
	item.goTo(kernel().lyxview());
}


vector<string> const ControlToc::getTypes() const
{
	return toc::getTypes(kernel().buffer());
}


string const ControlToc::getGuiName(string const & type) const
{
	if (type == "TOC")
		return _("Table of Contents");
	else
		return _(toc::getGuiName(type, kernel().buffer()));
}


toc::Toc const ControlToc::getContents(string const & type) const
{
	toc::Toc empty_list;

	// This shouldn't be possible...
	if (!kernel().isBufferAvailable()) {
		return empty_list;
	}

	toc::TocList tmp = toc::getTocList(kernel().buffer());
	toc::TocList::iterator it = tmp.find(type);
	if (it == tmp.end()) {
		return empty_list;
	}

	return it->second;
}

} // namespace frontend
} // namespace lyx
