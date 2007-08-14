/**
 * \file InsetBibitem.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Alejandro Aguilar Sierra
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "InsetBibitem.h"

#include "Biblio.h"
#include "Buffer.h"
#include "BufferView.h"
#include "DispatchResult.h"
#include "FuncRequest.h"
#include "Font.h"
#include "InsetIterator.h"
#include "Lexer.h"
#include "Paragraph.h"
#include "ParagraphList.h"

#include "support/lstrings.h"
#include "support/std_ostream.h"
#include "support/convert.h"


namespace lyx {

using support::prefixIs;

using std::max;
using std::string;
using std::auto_ptr;
using std::ostream;

int InsetBibitem::key_counter = 0;
docstring const key_prefix = from_ascii("key-");

InsetBibitem::InsetBibitem(InsetCommandParams const & p)
	: InsetCommand(p, "bibitem"), counter(1)
{
	if (getParam("key").empty())
		setParam("key", key_prefix + convert<docstring>(++key_counter));
}


auto_ptr<Inset> InsetBibitem::doClone() const
{
	auto_ptr<InsetBibitem> b(new InsetBibitem(params()));
	b->setCounter(counter);
	return auto_ptr<Inset>(b);
}


void InsetBibitem::doDispatch(Cursor & cur, FuncRequest & cmd)
{
	switch (cmd.action) {

	case LFUN_INSET_MODIFY: {
		InsetCommandParams p("bibitem");
		InsetCommandMailer::string2params("bibitem", to_utf8(cmd.argument()), p);
		if (p.getCmdName().empty()) {
			cur.noUpdate();
			break;
		}
		if (p["key"] != params()["key"])
			cur.bv().buffer()->changeRefsIfUnique(params()["key"],
						       p["key"], Inset::CITE_CODE);
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


void InsetBibitem::read(Buffer const & buf, Lexer & lex)
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
	return label.empty() ? convert<docstring>(counter) : label;
}


docstring const InsetBibitem::getScreenLabel(Buffer const &) const
{
	return getParam("key") + " [" + getBibLabel() + ']';
}


int InsetBibitem::plaintext(Buffer const &, odocstream & os,
			    OutputParams const &) const
{
	odocstringstream oss;
	oss << '[' << getCounter() << "] ";

	docstring const str = oss.str();
	os << str;

	return str.size();
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
	and without GUI. After thinking about this it is clear that no Font
	metrics should be used here, since these come from the gui. If we can't
	easily get the LaTeX font metrics we should make our own poor mans front
	metrics replacement, e.g. by hardcoding the metrics of the standard TeX
	font.
	*/
	Font font;

	ParagraphList::const_iterator it = buffer.paragraphs().begin();
	ParagraphList::const_iterator end = buffer.paragraphs().end();

	for (; it != end; ++it) {
		if (it->bibitem()) {
			docstring const label = it->bibitem()->getBibLabel();

			// FIXME: we can't be sure using the following that the GUI
			// version and the command-line version will give the same
			// result.
			//
			//int const wx = use_gui?
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

	return from_ascii("99");
}


void InsetBibitem::fillWithBibKeys(Buffer const & buf,
	std::vector<std::pair<std::string, docstring> > & keys,
	InsetIterator const & it) const
{
	string const key = to_utf8(getParam("key"));
	docstring const label = getParam("label");
	DocIterator doc_it(it); 
	doc_it.forwardPos();
	docstring const ref = doc_it.paragraph().asString(buf, false);
	docstring const info = label + biblio::TheBibliographyRef + ref;
	keys.push_back(std::pair<string, docstring>(key, info));
}

} // namespace lyx
