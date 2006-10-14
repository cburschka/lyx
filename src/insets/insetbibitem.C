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

#include "frontends/FontMetrics.h"

#include "support/lstrings.h"
#include "support/std_ostream.h"
#include "support/convert.h"

using lyx::docstring;
using lyx::support::prefixIs;

using std::max;
using std::string;
using std::auto_ptr;
using std::ostream;

int InsetBibitem::key_counter = 0;
string const key_prefix = "key-";

namespace lyx {
extern bool use_gui;
}

InsetBibitem::InsetBibitem(InsetCommandParams const & p)
	: InsetCommand(p, "bibitem"), counter(1)
{
	if (getContents().empty())
		setContents(key_prefix + convert<string>(++key_counter));
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
		InsetCommandParams p;
		InsetCommandMailer::string2params("bibitem", lyx::to_utf8(cmd.argument()), p);
		if (p.getCmdName().empty()) {
 			cur.noUpdate();
			break;
		}
		if (p.getContents() != params().getContents()) 
			cur.bv().buffer()->changeRefsIfUnique(params().getContents(),
						       p.getContents(), InsetBase::CITE_CODE);
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


// I'm sorry but this is still necessary because \bibitem is used also
// as a LyX 2.x command, and lyxlex is not enough smart to understand
// real LaTeX commands. Yes, that could be fixed, but would be a waste
// of time cause LyX3 won't use lyxlex anyway.  (ale)
void InsetBibitem::write(Buffer const &, std::ostream & os) const
{
	os << "\n\\bibitem ";
	if (!getOptions().empty())
		os << '[' << getOptions() << ']';
	os << '{' << getContents() << "}\n";
}


// This is necessary here because this is written without begin_inset
// This should be changed!!! (Jug)
void InsetBibitem::read(Buffer const &, LyXLex & lex)
{
	if (lex.eatLine())
		scanCommand(lex.getString());
	else
		lex.printError("InsetCommand: Parse error: `$$Token'");

	if (prefixIs(getContents(), key_prefix)) {
		int const key = convert<int>(getContents().substr(key_prefix.length()));
		key_counter = max(key_counter, key);
	}
}


docstring const InsetBibitem::getBibLabel() const
{
	// FIXME UNICODE
	return getOptions().empty() ?
		convert<docstring>(counter) :
		lyx::from_utf8(getOptions());
}


docstring const InsetBibitem::getScreenLabel(Buffer const &) const
{
	// FIXME UNICODE
	return lyx::from_utf8(getContents()) + " [" + getBibLabel() + ']';
}


int InsetBibitem::plaintext(Buffer const &, lyx::odocstream & os,
			    OutputParams const &) const
{
	os << '[' << getCounter() << "] ";
	return 0;
}


// ale070405
docstring const bibitemWidest(Buffer const & buffer)
{
	int w = 0;
	// Does look like a hack? It is! (but will change at 0.13)

	InsetBibitem const * bitem = 0;
	// FIXME: this font is used unitialized for now but should  be set to
	// a proportional font. Here is what Georg has to say about it:
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
            
			// FIXME 1: we can't be sure using the following that the GUI
			// version and the command-line version will give the same 
			// result.
			// FIXME 2: this use_gui test should be transfered to the frontend.
			int const wx = lyx::use_gui?
				theFontMetrics(font).width(label): label.size();

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
