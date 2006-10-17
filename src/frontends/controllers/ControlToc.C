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
	: ControlCommand(d, "tableofcontents", "toc")
{}


void ControlToc::goTo(toc::TocItem const & item)
{
	item.goTo(kernel().lyxview());
}


bool ControlToc::canOutline(string const & type)
{
	return type == "TOC";
}


void ControlToc::outlineUp()
{
	kernel().dispatch(FuncRequest(LFUN_OUTLINE_UP));
}


void ControlToc::outlineDown()
{
	kernel().dispatch(FuncRequest(LFUN_OUTLINE_DOWN));
}


void ControlToc::outlineIn()
{
	kernel().dispatch(FuncRequest(LFUN_OUTLINE_IN));
}


void ControlToc::outlineOut()
{
	kernel().dispatch(FuncRequest(LFUN_OUTLINE_OUT));
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
		return lyx::to_utf8(_("Table of Contents"));
	else
		return lyx::to_utf8(_(toc::getGuiName(type, kernel().buffer())));
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
