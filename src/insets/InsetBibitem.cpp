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

#include "Buffer.h"
#include "BufferParams.h"
#include "BufferView.h"
#include "Counters.h"
#include "DispatchResult.h"
#include "FuncRequest.h"
#include "InsetIterator.h"
#include "InsetList.h"
#include "Lexer.h"
#include "Paragraph.h"
#include "ParagraphList.h"
#include "TextClass.h"

#include "support/lstrings.h"
#include "support/docstream.h"
#include "support/convert.h"

#include <ostream>

using namespace std;
using namespace lyx::support;

namespace lyx {


int InsetBibitem::key_counter = 0;


docstring const key_prefix = from_ascii("key-");


InsetBibitem::InsetBibitem(InsetCommandParams const & p)
	: InsetCommand(p, "bibitem")
{
	if (getParam("key").empty())
		setParam("key", key_prefix + convert<docstring>(++key_counter));
}


CommandInfo const * InsetBibitem::findInfo(string const & /* cmdName */)
{
	static const char * const paramnames[] = {"label", "key", ""};
	static const bool isoptional[] = {true, false};
	static const CommandInfo info = {2, paramnames, isoptional};
	return &info;
}


Inset * InsetBibitem::clone() const
{
	InsetBibitem * b = new InsetBibitem(params());
	b->autolabel_ = autolabel_;
	return b;
}


void InsetBibitem::doDispatch(Cursor & cur, FuncRequest & cmd)
{
	switch (cmd.action) {

	case LFUN_INSET_MODIFY: {
		InsetCommandParams p(BIBITEM_CODE);
		InsetCommandMailer::string2params("bibitem", to_utf8(cmd.argument()), p);
		if (p.getCmdName().empty()) {
			cur.noUpdate();
			break;
		}
		if (p["key"] != params()["key"])
			cur.bv().buffer().changeRefsIfUnique(params()["key"],
						       p["key"], CITE_CODE);
		setParams(p);
	}

	default:
		InsetCommand::doDispatch(cur, cmd);
		break;
	}
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
	return label.empty() ? autolabel_ : label;
}


docstring const InsetBibitem::getScreenLabel(Buffer const &) const
{
	return getParam("key") + " [" + getBibLabel() + ']';
}


int InsetBibitem::plaintext(Buffer const &, odocstream & os,
			    OutputParams const &) const
{
	odocstringstream oss;
	oss << '[' << getBibLabel() << "] ";

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
	easily get the LaTeX font metrics we should make our own poor mans font
	metrics replacement, e.g. by hardcoding the metrics of the standard TeX
	font.
	*/

	ParagraphList::const_iterator it = buffer.paragraphs().begin();
	ParagraphList::const_iterator end = buffer.paragraphs().end();

	for (; it != end; ++it) {
		if (it->insetList().empty())
			continue;
		Inset * inset = it->insetList().begin()->inset;
		if (inset->lyxCode() != BIBITEM_CODE)
			continue;

		bitem = static_cast<InsetBibitem const *>(inset);
		docstring const label = bitem->getBibLabel();

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

		if (wx > w)
			w = wx;
	}

	if (bitem && !bitem->getBibLabel().empty())
		return bitem->getBibLabel();

	return from_ascii("99");
}


void InsetBibitem::fillWithBibKeys(Buffer const & buf,
	BiblioInfo & keys, InsetIterator const & it) const
{
	docstring const key = getParam("key");
	BibTeXInfo keyvalmap(false);
	keyvalmap[from_ascii("label")] = getParam("label");
	DocIterator doc_it(it); 
	doc_it.forwardPos();
	keyvalmap[from_ascii("ref")] = doc_it.paragraph().asString(buf, false);
	keys[key] = keyvalmap;
}


/// Update the counters of this inset and of its contents
void InsetBibitem::updateLabels(Buffer const &buf, ParIterator const &) 
{
	Counters & counters = buf.params().textClass().counters();
	docstring const bibitem = from_ascii("bibitem");
	if (counters.hasCounter(bibitem) && getParam("label").empty()) {
		counters.step(bibitem);
		autolabel_ = counters.theCounter(bibitem);
	} else
		autolabel_ = from_ascii("??");
	refresh();
}


} // namespace lyx
