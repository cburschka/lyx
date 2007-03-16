/**
 * \file ControlToc.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Angus Leeming
 * \author Abdelrazak Younes
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
{
}


TocList const & ControlToc::tocs() const
{
	return kernel().buffer().tocBackend().tocs();
}


bool ControlToc::initialiseParams(string const & data)
{
	if (!ControlCommand::initialiseParams(data))
		return false;

	types_.clear();
	type_names_.clear();
	TocList const & tocs = kernel().buffer().tocBackend().tocs();
	TocList::const_iterator it = tocs.begin();
	TocList::const_iterator end = tocs.end();
	for (; it != end; ++it) {
		types_.push_back(it->first);
		type_names_.push_back(getGuiName(it->first));
	}

	string selected_type ;
	if(params()["type"].empty()) //Then plain toc...
		selected_type = params().getCmdName();
	else
		selected_type = to_ascii(params()["type"]);
	selected_type_ = -1;
	for (size_t i = 0;  i != types_.size(); ++i) {
		if (selected_type == types_[i]) {
			selected_type_ = i;
			break;
		}
	}

	update();
	return true;
}

void ControlToc::goTo(TocItem const & item)
{
	string const tmp = convert<string>(item.id());
	kernel().lyxview().dispatch(FuncRequest(LFUN_PARAGRAPH_GOTO, tmp));
}


bool ControlToc::canOutline(size_t type) const
{
	BOOST_ASSERT(type >= 0 && type < int(types_.size()));
	return types_[type] == "tableofcontents";
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


void ControlToc::updateBackend()
{
	kernel().buffer().tocBackend().update();
}


TocIterator const ControlToc::getCurrentTocItem(size_t type) const
{
	BOOST_ASSERT(kernel().bufferview());
	ParConstIterator it(kernel().bufferview()->cursor());
	return kernel().buffer().tocBackend().item(types_[type], it);
}


docstring const ControlToc::getGuiName(string const & type) const
{
	if (type == "tableofcontents")
		return _("Table of Contents");

	FloatList const & floats =
		kernel().buffer().params().getLyXTextClass().floats();
	if (floats.typeExist(type))
		return from_utf8(floats.getType(type).name());
	else
		return _(type);
}

} // namespace frontend
} // namespace lyx
