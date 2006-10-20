/**
 * \file insetbibitem.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Alejandro Aguilar Sierra
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "insetbibitem.h"

#include "buffer.h"
#include "BufferView.h"
#include "dispatchresult.h"
#include "funcrequest.h"
#include "lyxfont.h"
#include "lyxlex.h"
#include "paragraph.h"
#include "ParagraphList.h"

#include "support/lstrings.h"
#include "support/std_ostream.h"
#include "support/convert.h"

using lyx::docstring;
using lyx::odocstream;
using lyx::support::prefixIs;

using std::max;
using std::string;
using std::auto_ptr;
using std::ostream;

int InsetBibitem::key_counter = 0;
docstring const key_prefix = lyx::from_ascii("key-");

InsetBibitem::InsetBibitem(InsetCommandParams const & p)
	: InsetCommand(p, "bibitem"), counter(1)
{
	if (getParam("key").empty())
		setParam("key", key_prefix + convert<docstring>(++key_counter));
}


auto_ptr<InsetBase> InsetBibitem::doClone() const
{
	auto_ptr<InsetBibitem> b(new InsetBibitem(params()));
	b->setCounter(counter);
	return auto_ptr<InsetBase>(b);
}


void InsetBibitem::doDispatch(LCursor & cur, FuncRequest & cmd)
{
	switch (cmd.action) {

	case LFUN_INSET_MODIFY: {
		InsetCommandParams p("bibitem");
		InsetCommandMailer::string2params("bibitem", lyx::to_utf8(cmd.argument()), p);
		if (p.getCmdName().empty()) {
 			cur.noUpdate();
			break;
		}
		if (p["key"] != params()["key"])
			// FIXME UNICODE
			cur.bv().buffer()->changeRefsIfUnique(lyx::to_utf8(params()["key"]),
						       lyx::to_utf8(p["key"]), InsetBase::CITE_CODE);
		setParams(p);
	}

	default:
		InsetCommand::doDispatch(cur, cmd);
		break;
	}
}


void InsetBibitem::setCounter(int c)
{
	counter = c;
}


void InsetBibitem::read(Buffer const & buf, LyXLex & lex)
{
	InsetCommand::read(buf, lex);

	if (prefixIs(getParam("key"), key_prefix)) {
		int const key = convert<int>(getParam("key").substr(key_prefix.length()));
		key_counter = max(key_counter, key);
	}
}


docstring const InsetBibitem::getBibLabel() const
{
	docstring const & label = getParam("label");
	return label.empty() ?  convert<docstring>(counter) : label;
}


docstring const InsetBibitem::getScreenLabel(Buffer const &) const
{
	return getParam("key") + " [" + getBibLabel() + ']';
}


int InsetBibitem::plaintext(Buffer const &, odocstream & os,
			    OutputParams const &) const
{
	os << '[' << getCounter() << "] ";
	return 0;
}


// ale070405
docstring const bibitemWidest(Buffer const & buffer)
{
	int w = 0;

	InsetBibitem const * bitem = 0;

	// FIXME: this font is used unitialized for now but should  be set to
	// a proportional font. Here is what Georg Baum has to say about it:
	/*
	bibitemWidest() is supposed to find the bibitem with the widest label in the 
	output, because that is needed as an argument of the bibliography 
	environment to dtermine the correct indentation. To be 100% correct we 
	would need the metrics of the font that is used in the output, but usually 
	we don't have access to these.
	In practice, any proportional font is probably good enough, since we don't 
	need to know the final with, we only need to know the which label is the 
	widest.
	Unless there is an easy way to get the metrics of the output font I suggest 
	to use a hardcoded font like "Times" or so.

	It is very important that the result of this function is the same both with 
	and without GUI. After thinking about this it is clear that no LyXFont 
	metrics should be used here, since these come from the gui. If we can't 
	easily get the LaTeX font metrics we should make our own poor mans front 
	metrics replacement, e.g. by hardcoding the metrics of the standard TeX 
	font.
	*/
	LyXFont font;

	ParagraphList::const_iterator it = buffer.paragraphs().begin();
	ParagraphList::const_iterator end = buffer.paragraphs().end();

	for (; it != end; ++it) {
		if (it->bibitem()) {
			docstring const label = it->bibitem()->getBibLabel();
            
			// FIXME: we can't be sure using the following that the GUI
			// version and the command-line version will give the same 
			// result.
			//
			//int const wx = lyx::use_gui?
			//	theFontMetrics(font).width(label): label.size();
			//
			// So for now we just use the label size in order to be sure
			// that GUI and no-GUI gives the same bibitem (even if that is 
			// potentially the wrong one.
			int const wx = label.size();

			if (wx > w) {
				w = wx;
				bitem = it->bibitem();
			}
		}
	}

	if (bitem && !bitem->getBibLabel().empty())
		return bitem->getBibLabel();

	return lyx::from_ascii("99");
}
