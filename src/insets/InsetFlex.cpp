/**
 * \file InsetFlex.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Angus Leeming
 * \author Martin Vermeer
 * \author Jürgen Spitzmüller
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "InsetFlex.h"

#include "Buffer.h"
#include "BufferParams.h"
#include "BufferView.h"
#include "DispatchResult.h"
#include "FuncRequest.h"
#include "FuncStatus.h"
#include "Cursor.h"
#include "gettext.h"
#include "Lexer.h"
#include "Text.h"
#include "MetricsInfo.h"
#include "Paragraph.h"
#include "paragraph_funcs.h"
#include "sgml.h"

#include "frontends/FontMetrics.h"
#include "frontends/Painter.h"

#include "support/convert.h"

#include <sstream>


namespace lyx {

using std::string;
using std::ostream;


InsetFlex::InsetFlex(BufferParams const & bp,
				InsetLayout il)
	: InsetCollapsable(bp, Collapsed)
{
	params_.name = il.name;
	setLayout(il);
}


InsetFlex::InsetFlex(InsetFlex const & in)
	: InsetCollapsable(in), params_(in.params_)
{}


Inset * InsetFlex::clone() const
{
	return new InsetFlex(*this);
}


bool InsetFlex::undefined() const
{
	return layout_.labelstring == from_utf8("UNDEFINED");
}


void InsetFlex::setLayout(InsetLayout il)
{
	layout_ = il;
}


docstring const InsetFlex::editMessage() const
{
	return _("Opened Flex Inset");
}


void InsetFlex::write(Buffer const & buf, ostream & os) const
{
	params_.write(os);
	InsetCollapsable::write(buf, os);
}


void InsetFlex::read(Buffer const & buf, Lexer & lex)
{
	params_.read(lex);
	InsetCollapsable::read(buf, lex);
}


void InsetFlex::metrics(MetricsInfo & mi, Dimension & dim) const
{
	FontInfo tmpfont = mi.base.font;
	getDrawFont(mi.base.font);
	mi.base.font.reduce(sane_font);
	mi.base.font.realize(tmpfont);
	InsetCollapsable::metrics(mi, dim);
	mi.base.font = tmpfont;
}


void InsetFlex::draw(PainterInfo & pi, int x, int y) const
{
	FontInfo tmpfont = pi.base.font;
	getDrawFont(pi.base.font);
	// I don't understand why the above .reduce and .realize aren't
	//needed, or even wanted, here. It just works. -- MV 10.04.2005
	InsetCollapsable::draw(pi, x, y);
	pi.base.font = tmpfont;
}


void InsetFlex::getDrawFont(FontInfo & font) const
{
	font = layout_.font;
}


void InsetFlex::doDispatch(Cursor & cur, FuncRequest & cmd)
{
	InsetCollapsable::doDispatch(cur, cmd);
}


bool InsetFlex::getStatus(Cursor & cur, FuncRequest const & cmd,
	FuncStatus & status) const
{
	switch (cmd.action) {
		// paragraph breaks not allowed in flex insets
		case LFUN_BREAK_PARAGRAPH:
		case LFUN_BREAK_PARAGRAPH_SKIP:
			status.enabled(layout_.multipar);
			return true;

		default:
			return InsetCollapsable::getStatus(cur, cmd, status);
		}
}


int InsetFlex::plaintext(Buffer const & buf, odocstream & os,
			      OutputParams const & runparams) const
{
	return InsetText::plaintext(buf, os, runparams);
}


int InsetFlex::docbook(Buffer const & buf, odocstream & os,
			    OutputParams const & runparams) const
{
	ParagraphList::const_iterator beg = paragraphs().begin();
	ParagraphList::const_iterator par = paragraphs().begin();
	ParagraphList::const_iterator end = paragraphs().end();

	if (!undefined())
		// FIXME UNICODE
		sgml::openTag(os, layout_.latexname,
			      par->getID(buf, runparams) + layout_.latexparam);

	for (; par != end; ++par) {
		par->simpleDocBookOnePar(buf, os, runparams,
					 outerFont(std::distance(beg, par),
						   paragraphs()));
	}

	if (!undefined())
		sgml::closeTag(os, layout_.latexname);

	return 0;
}


void InsetFlex::textString(Buffer const & buf, odocstream & os) const
{
	os << paragraphs().begin()->asString(buf, true);
}


void InsetFlexParams::write(ostream & os) const
{
	os << "Flex " << name << "\n";
}


void InsetFlexParams::read(Lexer & lex)
{
	while (lex.isOK()) {
		lex.next();
		string token = lex.getString();

		if (token == "Flex") {
			lex.next();
			name = lex.getString();
		}

		// This is handled in Collapsable
		else if (token == "status") {
			lex.pushToken(token);
			break;
		}
	}
}


} // namespace lyx
