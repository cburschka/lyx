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
#include "buffer.h"
#include "BufferView.h"
#include "bufferparams.h"
#include "debug.h"
#include "FloatList.h"
#include "funcrequest.h"
#include "gettext.h"

#include "frontends/LyXView.h"

#include "support/convert.h"

using std::vector;
using std::string;


class Buffer;

namespace lyx {
namespace frontend {


ControlToc::ControlToc(Dialog & d)
	: ControlCommand(d, "tableofcontents", "toc")
{}


void ControlToc::goTo(TocBackend::Item const & item)
{
	string const tmp = convert<string>(item.id());
	kernel().lyxview().dispatch(FuncRequest(LFUN_PARAGRAPH_GOTO, tmp));
}


bool ControlToc::canOutline(string const & type)
{
	return type == "tableofcontents";
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
	return kernel().buffer().tocBackend().types();
}


TocBackend::Toc::const_iterator const ControlToc::getCurrentTocItem(
	string const & type) const
{
	BOOST_ASSERT(kernel().bufferview());

	ParConstIterator it(kernel().bufferview()->cursor());
	return kernel().buffer().tocBackend().item(type, it);
}


string const ControlToc::getGuiName(string const & type) const
{
	if (type == "tableofcontents")
		return lyx::to_utf8(_("Table of Contents"));

	FloatList const & floats =
		kernel().buffer().params().getLyXTextClass().floats();
	if (floats.typeExist(type))
		return floats.getType(type).name();
	else
		return lyx::to_utf8(_(type));
}


TocBackend::Toc const empty_list;

TocBackend::Toc const & ControlToc::getContents(string const & type) const
{
	// This shouldn't be possible...
	if (!kernel().isBufferAvailable()) {
		return empty_list;
	}

	return kernel().buffer().tocBackend().toc(type);
}

} // namespace frontend
} // namespace lyx
