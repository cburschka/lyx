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

#include "frontends/font_metrics.h"

#include "support/lstrings.h"
#include "support/tostr.h"

using lyx::support::prefixIs;
using lyx::support::strToInt;

using std::max;
using std::string;
using std::auto_ptr;


int InsetBibitem::key_counter = 0;
string const key_prefix = "key-";


InsetBibitem::InsetBibitem(InsetCommandParams const & p)
	: InsetCommand(p, "bibitem"), counter(1)
{
	if (getContents().empty())
		setContents(key_prefix + tostr(++key_counter));
}


auto_ptr<InsetBase> InsetBibitem::clone() const
{
	auto_ptr<InsetBibitem> b(new InsetBibitem(params()));
	b->setCounter(counter);
	return auto_ptr<InsetBase>(b);
}


void InsetBibitem::priv_dispatch(LCursor & cur, FuncRequest const & cmd)
{
	switch (cmd.action) {

	case LFUN_INSET_MODIFY: {
		InsetCommandParams p;
		InsetCommandMailer::string2params("bibitem", cmd.argument, p);
		if (p.getCmdName().empty())
			break;
		setParams(p);
		cur.bv().update();
		cur.bv().fitCursor();
		break;
	}

	default:
		InsetCommand::priv_dispatch(cur, cmd);
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
	if (lex.eatLine()) {
		scanCommand(lex.getString());
	} else {
		lex.printError("InsetCommand: Parse error: `$$Token'");
	}

	if (prefixIs(getContents(), key_prefix)) {
		int key = strToInt(getContents().substr(key_prefix.length()));
		key_counter = max(key_counter, key);
	}
}


string const InsetBibitem::getBibLabel() const
{
	return getOptions().empty() ? tostr(counter) : getOptions();
}


string const InsetBibitem::getScreenLabel(Buffer const &) const
{
	return getContents() + " [" + getBibLabel() + ']';
}


// ale070405 This function maybe shouldn't be here. We'll fix this at 0.13.
int bibitemMaxWidth(BufferView * bv, LyXFont const &)
{
	int w = 0;
	// Ha, now we are mainly at 1.2.0 and it is still here (Jug)
	// Does look like a hack? It is! (but will change at 0.13)
	ParagraphList::iterator it = bv->buffer()->paragraphs().begin();
	ParagraphList::iterator end = bv->buffer()->paragraphs().end();
	for (; it != end; ++it) {
		if (it->bibitem()) {
#warning metrics broken!
			int const wx = it->bibitem()->width();
			if (wx > w)
				w = wx;
		}
	}
	return w;
}


// ale070405
string const bibitemWidest(Buffer const & buffer)
{
	int w = 0;
	// Does look like a hack? It is! (but will change at 0.13)

	InsetBibitem const * bitem = 0;
	LyXFont font;

	ParagraphList::const_iterator it = buffer.paragraphs().begin();
	ParagraphList::const_iterator end = buffer.paragraphs().end();

	for (; it != end; ++it) {
		if (it->bibitem()) {
			int const wx =
				font_metrics::width(it->bibitem()->getBibLabel(),
						    font);
			if (wx > w) {
				w = wx;
				bitem = it->bibitem();
			}
		}
	}

	if (bitem && !bitem->getBibLabel().empty())
		return bitem->getBibLabel();

	return "99";
}
