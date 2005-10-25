/**
 * \file toc.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Jean-Marc Lasgouttes
 * \author Angus Leeming
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "toc.h"

#include "buffer.h"
#include "bufferparams.h"
#include "FloatList.h"
#include "funcrequest.h"
#include "LyXAction.h"
#include "paragraph.h"
#include "pariterator.h"

#include "frontends/LyXView.h"

#include "insets/insetfloat.h"
#include "insets/insetoptarg.h"
#include "insets/insetwrap.h"

#include "support/convert.h"

using std::vector;
using std::max;
using std::ostream;
using std::string;

namespace lyx {
namespace toc {

string const TocItem::asString() const
{
	return string(4 * depth, ' ') + str;
}


void TocItem::goTo(LyXView & lv_) const
{
	string const tmp = convert<string>(id_);
	lv_.dispatch(FuncRequest(LFUN_GOTO_PARAGRAPH, tmp));
}


FuncRequest TocItem::action() const
{
	return FuncRequest(LFUN_GOTO_PARAGRAPH, convert<string>(id_));
}


string const getType(string const & cmdName)
{
	// special case
	if (cmdName == "tableofcontents")
		return "TOC";
	else
		return cmdName;
}


string const getGuiName(string const & type, Buffer const & buffer)
{
	FloatList const & floats =
		buffer.params().getLyXTextClass().floats();
	if (floats.typeExist(type))
		return floats.getType(type).name();
	else
		return type;
}


TocList const getTocList(Buffer const & buf)
{
	TocList toclist;

	BufferParams const & bufparams = buf.params();
	const int min_toclevel = bufparams.getLyXTextClass().min_toclevel();

	ParConstIterator pit = buf.par_iterator_begin();
	ParConstIterator end = buf.par_iterator_end();
	for (; pit != end; ++pit) {

		// the string that goes to the toc (could be the optarg)
		string tocstring;

		// For each paragraph, traverse its insets and look for
		// FLOAT_CODE or WRAP_CODE
		InsetList::const_iterator it = pit->insetlist.begin();
		InsetList::const_iterator end = pit->insetlist.end();
		for (; it != end; ++it) {
			switch (it->inset->lyxCode()) {
			case InsetBase::FLOAT_CODE:
				static_cast<InsetFloat*>(it->inset)
					->addToToc(toclist, buf);
				break;
			case InsetBase::WRAP_CODE:
				static_cast<InsetWrap*>(it->inset)
					->addToToc(toclist, buf);
				break;
			case InsetBase::OPTARG_CODE: {
				if (!tocstring.empty())
					break;
				Paragraph const & par = *static_cast<InsetOptArg*>(it->inset)->paragraphs().begin();
				if (!pit->getLabelstring().empty())
					tocstring = pit->getLabelstring()
						+ ' ';
				tocstring += par.asString(buf, false);
				break;
			}
			default:
				break;
			}
		}

		/// now the toc entry for the paragraph
		int const toclevel = pit->layout()->toclevel;
		if (toclevel != LyXLayout::NOT_IN_TOC
		    && toclevel >= min_toclevel
		    && toclevel <= bufparams.tocdepth) {
			// insert this into the table of contents
			if (tocstring.empty())
				tocstring = pit->asString(buf, true);
			TocItem const item(pit->id(), toclevel - min_toclevel,
					   tocstring);
			toclist["TOC"].push_back(item);
		}
	}
	return toclist;
}


vector<string> const getTypes(Buffer const & buffer)
{
	vector<string> types;

	TocList const tmp = getTocList(buffer);

	TocList::const_iterator cit = tmp.begin();
	TocList::const_iterator end = tmp.end();

	for (; cit != end; ++cit) {
		types.push_back(cit->first);
	}

	return types;
}


void asciiTocList(string const & type, Buffer const & buffer, ostream & os)
{
	TocList const toc_list = getTocList(buffer);
	TocList::const_iterator cit = toc_list.find(type);
	if (cit != toc_list.end()) {
		Toc::const_iterator ccit = cit->second.begin();
		Toc::const_iterator end = cit->second.end();
		for (; ccit != end; ++ccit)
			os << ccit->asString() << '\n';
	}
}


} // namespace toc
} // namespace lyx
