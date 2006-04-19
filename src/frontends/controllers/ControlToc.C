/**
 * \file ControlToc.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Angus Leeming
 *
 * Full author contact details are available in file CREDITS.
 */

#include <sstream>

#include <config.h>

#include "ControlToc.h"
#include "funcrequest.h"
#include "gettext.h"
#include "BufferView.h"

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


bool ControlToc::canOutline(string const & type)
{
	return type == "TOC";
}


void ControlToc::outline(toc::OutlineOp op)
{
	std::ostringstream o;
	o << op << std::flush;
	kernel().dispatch(FuncRequest(LFUN_OUTLINE, o.str()));
}


vector<string> const ControlToc::getTypes() const
{
	return toc::getTypes(kernel().buffer());
}


toc::TocItem const ControlToc::getCurrentTocItem(
	string const & type) const
{
	BufferView const * const bv = kernel().bufferview();
	if (!bv)
		return toc::TocItem(-1, -1, "");

	return toc::getCurrentTocItem(kernel().buffer(), bv->cursor(), type);
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
