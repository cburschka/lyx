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
#include "debug.h"

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


vector<string> const & ControlToc::getTypes() const
{
	return toc::getTypes(kernel().buffer());
}


toc::TocIterator const ControlToc::getCurrentTocItem(
	string const & type) const
{
	BOOST_ASSERT(kernel().bufferview());

	return toc::getCurrentTocItem(kernel().buffer(),
		kernel().bufferview()->cursor(), type);
}


string const ControlToc::getGuiName(string const & type) const
{
	if (type == "TOC")
		return _("Table of Contents");
	else
		return _(toc::getGuiName(type, kernel().buffer()));
}


toc::Toc const empty_list;

toc::Toc const & ControlToc::getContents(string const & type) const
{
	// This shouldn't be possible...
	if (!kernel().isBufferAvailable()) {
		return empty_list;
	}

	return toc::getToc(kernel().buffer(), type);
}

} // namespace frontend
} // namespace lyx
