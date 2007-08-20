/**
 * \file InsetCharStyle.cpp
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

#include "InsetCharStyle.h"

#include "Buffer.h"
#include "BufferParams.h"
#include "BufferView.h"
#include "DispatchResult.h"
#include "FuncRequest.h"
#include "FuncStatus.h"
#include "Cursor.h"
#include "gettext.h"
#include "LaTeXFeatures.h"
#include "Color.h"
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
using std::auto_ptr;
using std::istringstream;
using std::max;
using std::ostream;
using std::ostringstream;


void InsetCharStyle::init()
{}


InsetCharStyle::InsetCharStyle(BufferParams const & bp, string const s)
	: InsetCollapsable(bp, Collapsed)
{
	params_.name = s;
	setUndefined();
	init();
}


InsetCharStyle::InsetCharStyle(BufferParams const & bp,
				CharStyles::iterator cs)
	: InsetCollapsable(bp, Collapsed)
{
	params_.name = cs->name;
	setDefined(cs);
	init();
}


InsetCharStyle::InsetCharStyle(InsetCharStyle const & in)
	: InsetCollapsable(in), params_(in.params_)
{
	init();
}


auto_ptr<Inset> InsetCharStyle::doClone() const
{
	return auto_ptr<Inset>(new InsetCharStyle(*this));
}


bool InsetCharStyle::undefined() const
{
	return layout_.latexname.empty();
}


void InsetCharStyle::setUndefined()
{
	layout_.latextype.clear();
	layout_.latexname.clear();
	layout_.latexparam.clear();
	layout_.font = Font(Font::ALL_INHERIT);
	layout_.labelfont = Font(Font::ALL_INHERIT);
	layout_.labelfont.setColor(Color::error);
}


void InsetCharStyle::setDefined(CharStyles::iterator cs)
{
	layout_ = *cs;
}


docstring const InsetCharStyle::editMessage() const
{
	return _("Opened CharStyle Inset");
}


void InsetCharStyle::write(Buffer const & buf, ostream & os) const
{
	params_.write(os);
	InsetCollapsable::write(buf, os);
}


void InsetCharStyle::read(Buffer const & buf, Lexer & lex)
{
	params_.read(lex);
	InsetCollapsable::read(buf, lex);
}


bool InsetCharStyle::metrics(MetricsInfo & mi, Dimension & dim) const
{
	Font tmpfont = mi.base.font;
	getDrawFont(mi.base.font);
	mi.base.font.reduce(Font(Font::ALL_SANE));
	mi.base.font.realize(tmpfont);
	bool changed = InsetCollapsable::metrics(mi, dim);
	mi.base.font = tmpfont;
	if (status() == Open) {
		// FIXME UNICODE
		docstring s(from_utf8(params_.name));
		if (undefined())
			s = _("Undef: ") + s;
		// Chop off prefix:
		if (s.find(':') != string::npos)
			s = s.substr(s.find(':'));
		layout_.labelstring = s;
	}
	return changed;
}


void InsetCharStyle::draw(PainterInfo & pi, int x, int y) const
{
	setPosCache(pi, x, y);

	Font tmpfont = pi.base.font;
	getDrawFont(pi.base.font);
	// I don't understand why the above .reduce and .realize aren't
	//needed, or even wanted, here. It just works. -- MV 10.04.2005
	InsetCollapsable::draw(pi, x, y);
	pi.base.font = tmpfont;

	// the name of the charstyle. Can be toggled.
	if (status() == Open) {
		// FIXME UNICODE
		docstring s(from_utf8(params_.name));
		if (undefined())
			s = _("Undef: ") + s;
		// Chop off prefix:
		if (s.find(':') != string::npos)
			s = s.substr(s.find(':'));
		layout_.labelstring = s;
	}
}


void InsetCharStyle::getDrawFont(Font & font) const
{
	font = layout_.font;
}


void InsetCharStyle::doDispatch(Cursor & cur, FuncRequest & cmd)
{
	InsetCollapsable::doDispatch(cur, cmd);
}


bool InsetCharStyle::getStatus(Cursor & cur, FuncRequest const & cmd,
	FuncStatus & status) const
{
	switch (cmd.action) {
		// paragraph breaks not allowed in charstyle insets
		case LFUN_BREAK_PARAGRAPH:
		case LFUN_BREAK_PARAGRAPH_KEEP_LAYOUT:
		case LFUN_BREAK_PARAGRAPH_SKIP:
			status.enabled(false);
			return true;

		default:
			return InsetCollapsable::getStatus(cur, cmd, status);
		}
}


int InsetCharStyle::plaintext(Buffer const & buf, odocstream & os,
			      OutputParams const & runparams) const
{
	return InsetText::plaintext(buf, os, runparams);
}


int InsetCharStyle::docbook(Buffer const & buf, odocstream & os,
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


void InsetCharStyle::textString(Buffer const & buf, odocstream & os) const
{
	os << paragraphs().begin()->asString(buf, true);
}


void InsetCharStyle::validate(LaTeXFeatures & features) const
{
	// Force inclusion of preamble snippet in layout file
	features.require(layout_.latexname);
	InsetText::validate(features);
}


void InsetCharStyleParams::write(ostream & os) const
{
	os << "CharStyle " << name << "\n";
}


void InsetCharStyleParams::read(Lexer & lex)
{
	while (lex.isOK()) {
		lex.next();
		string token = lex.getString();

		if (token == "CharStyle") {
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
