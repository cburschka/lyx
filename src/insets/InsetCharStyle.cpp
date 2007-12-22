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
{
	setInlined();
	setDrawFrame(false);
}


InsetCharStyle::InsetCharStyle(BufferParams const & bp, string const s)
	: InsetCollapsable(bp), mouse_hover_(false)
{
	params_.type = s;
	setUndefined();
	init();
}


InsetCharStyle::InsetCharStyle(BufferParams const & bp,
				CharStyles::iterator cs)
	: InsetCollapsable(bp), mouse_hover_(false)
{
	params_.type = cs->name;
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
	return params_.latexname.empty();
}


void InsetCharStyle::setUndefined()
{
	params_.latextype.clear();
	params_.latexname.clear();
	params_.latexparam.clear();
	params_.font = Font(Font::ALL_INHERIT);
	params_.labelfont = Font(Font::ALL_INHERIT);
	params_.labelfont.setColor(Color::error);
	params_.show_label = false;
}


void InsetCharStyle::setDefined(CharStyles::iterator cs)
{
	params_.latextype = cs->latextype;
	params_.latexname = cs->latexname;
	params_.latexparam = cs->latexparam;
	params_.font = cs->font;
	params_.labelfont = cs->labelfont;
	params_.show_label = false;
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
	setInlined();
}


bool InsetCharStyle::metrics(MetricsInfo & mi, Dimension & dim) const
{
	Font tmpfont = mi.base.font;
	getDrawFont(mi.base.font);
	mi.base.font.reduce(Font(Font::ALL_SANE));
	mi.base.font.realize(tmpfont);
	InsetText::metrics(mi, dim);
	mi.base.font = tmpfont;
	if (params_.show_label) {
		// consider width of the inset label
		Font font(params_.labelfont);
		font.realize(Font(Font::ALL_SANE));
		font.decSize();
		font.decSize();
		int w = 0;
		int a = 0;
		int d = 0;
		// FIXME UNICODE
		docstring s(from_utf8(params_.type));
		if (undefined())
			s = _("Undef: ") + s;
		theFontMetrics(font).rectText(s, w, a, d);
		dim.wid = max(dim.wid, w);
	}
	if (params_.show_label)
		dim.des += ascent();
	bool const changed = dim_ != dim;
	dim_ = dim;
	return changed;
}


void InsetCharStyle::draw(PainterInfo & pi, int x, int y) const
{
	setPosCache(pi, x, y);

	Font tmpfont = pi.base.font;
	getDrawFont(pi.base.font);
	// I don't understand why the above .reduce and .realize aren't
	//needed, or even wanted, here. It just works. -- MV 10.04.2005
	InsetText::draw(pi, x, y);
	pi.base.font = tmpfont;

	int desc = InsetText::descent();
	if (params_.show_label)
		desc -= ascent();

	pi.pain.line(x, y + desc - 4, x, y + desc, params_.labelfont.color());
	pi.pain.line(x, y + desc, x + dim_.wid - 3, y + desc,
		params_.labelfont.color());
	pi.pain.line(x + dim_.wid - 3, y + desc, x + dim_.wid - 3, y + desc - 4,
		params_.labelfont.color());

	// the name of the charstyle. Can be toggled.
	// FIXME UNICODE
	docstring s(from_utf8(params_.type));
	if (undefined())
		s = _("Undef: ") + s;
	if (params_.show_label) {
		Font font(params_.labelfont);
		font.realize(Font(Font::ALL_SANE));
		font.decSize();
		font.decSize();
		int w = 0;
		int a = 0;
		int d = 0;
		theFontMetrics(font).rectText(s, w, a, d);
		pi.pain.rectText(x + (dim_.wid - w) / 2, y + desc + a,
			s, font, Color::none, Color::none);
	}

	// a visual clue when the cursor is inside the inset
	Cursor & cur = pi.base.bv->cursor();
	if (mouse_hover_ || cur.isInside(this)) {
		y -= ascent();
		pi.pain.line(x, y + 4, x, y, params_.labelfont.color());
		pi.pain.line(x + 4, y, x, y, params_.labelfont.color());
		pi.pain.line(x + dim_.wid - 3, y + 4, x + dim_.wid - 3, y,
			params_.labelfont.color());
		pi.pain.line(x + dim_.wid - 7, y, x + dim_.wid - 3, y,
			params_.labelfont.color());

		pi.base.bv->message(_("CharStyle: ") + s);
	}
}


void InsetCharStyle::getDrawFont(Font & font) const
{
	font = params_.font;
}


void InsetCharStyle::doDispatch(Cursor & cur, FuncRequest & cmd)
{
	setInlined();
	switch (cmd.action) {

	case LFUN_MOUSE_RELEASE:
			if (cmd.button() == mouse_button::button3)
				params_.show_label = !params_.show_label;
			else
				InsetCollapsable::doDispatch(cur, cmd);
			break;

	case LFUN_INSET_TOGGLE:
		if (cmd.argument() == "open")
			params_.show_label = true;
		else if (cmd.argument() == "close")
			params_.show_label = false;
		else if (cmd.argument() == "toggle" || cmd.argument().empty())
			params_.show_label = !params_.show_label;
		else // if assign or anything else
			cur.undispatched();
		cur.dispatched();
		break;

	default:
		InsetCollapsable::doDispatch(cur, cmd);
		break;
	}
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


int InsetCharStyle::latex(Buffer const & buf, odocstream & os,
			  OutputParams const & runparams) const
{
	if (!undefined()) {
		// FIXME UNICODE
		os << '\\' << from_utf8(params_.latexname);
		if (!params_.latexparam.empty())
			os << from_utf8(params_.latexparam);
		os << '{';
	}
	int i = InsetText::latex(buf, os, runparams);
	if (!undefined())
		os << "}";
	return i;
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
		sgml::openTag(os, params_.latexname,
			      par->getID(buf, runparams) + params_.latexparam);

	for (; par != end; ++par) {
		par->simpleDocBookOnePar(buf, os, runparams,
					 outerFont(std::distance(beg, par),
						   paragraphs()));
	}

	if (!undefined())
		sgml::closeTag(os, params_.latexname);

	return 0;
}


void InsetCharStyle::textString(Buffer const & buf, odocstream & os) const
{
	os << paragraphs().begin()->asString(buf, true);
}


void InsetCharStyle::validate(LaTeXFeatures & features) const
{
	// Force inclusion of preamble snippet in layout file
	features.require(params_.type);
	InsetText::validate(features);
}


bool InsetCharStyle::setMouseHover(bool mouse_hover)
{
	mouse_hover_ = mouse_hover;
	return true;
}


void InsetCharStyleParams::write(ostream & os) const
{
	os << "CharStyle " << type << "\n";
	os << "show_label " << convert<string>(show_label) << "\n";
}


void InsetCharStyleParams::read(Lexer & lex)
{
	while (lex.isOK()) {
		lex.next();
		string token = lex.getString();

		if (token == "CharStyle") {
			lex.next();
			type = lex.getString();
		}

		else if (token == "show_label") {
			lex.next();
			show_label = lex.getBool();
		}

		else if (token == "status") {
			lex.pushToken(token);
			break;
		}
	}
}


} // namespace lyx
