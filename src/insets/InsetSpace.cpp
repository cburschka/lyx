/**
 * \file InsetSpace.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Asger Alstrup Nielsen
 * \author Jean-Marc Lasgouttes
 * \author Lars Gullik Bjønnes
 * \author Jürgen Spitzmüller
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "InsetSpace.h"

#include "Cursor.h"
#include "Dimension.h"
#include "FuncRequest.h"
#include "FuncStatus.h"
#include "Length.h"
#include "Lexer.h"
#include "MetricsInfo.h"
#include "OutputParams.h"

#include "frontends/FontMetrics.h"
#include "frontends/Painter.h"

#include "support/debug.h"
#include "support/docstream.h"
#include "support/gettext.h"
#include "support/lstrings.h"

using namespace std;

namespace lyx {


InsetSpace::InsetSpace()
{}


InsetSpace::InsetSpace(InsetSpaceParams par)
{
	params_.kind = par.kind;
	params_.length = par.length;
}


InsetSpaceParams::Kind InsetSpace::kind() const
{
	return params_.kind;
}


Length InsetSpace::length() const
{
	return params_.length;
}


InsetSpace::~InsetSpace()
{
	InsetSpaceMailer(*this).hideDialog();
}


docstring InsetSpace::toolTip(BufferView const &, int, int) const
{
	docstring message;
	switch (params_.kind) {
	case InsetSpaceParams::NORMAL:
		message = _("Interword Space");
		break;
	case InsetSpaceParams::PROTECTED:
		message = _("Protected Space");
		break;
	case InsetSpaceParams::THIN:
		message = _("Thin Space");
		break;
	case InsetSpaceParams::QUAD:
		message = _("Quad Space");
		break;
	case InsetSpaceParams::QQUAD:
		message = _("QQuad Space");
		break;
	case InsetSpaceParams::ENSPACE:
		message = _("Enspace");
		break;
	case InsetSpaceParams::ENSKIP:
		message = _("Enskip");
		break;
	case InsetSpaceParams::NEGTHIN:
		message = _("Negative Thin Space");
		break;
	case InsetSpaceParams::HFILL:
		message = _("Horizontal Fill");
		break;
	case InsetSpaceParams::HFILL_PROTECTED:
		message = _("Protected Horizontal Fill");
		break;
	case InsetSpaceParams::DOTFILL:
		message = _("Horizontal Fill (Dots)");
		break;
	case InsetSpaceParams::HRULEFILL:
		message = _("Horizontal Fill (Rule)");
		break;
	case InsetSpaceParams::CUSTOM:
		message = support::bformat(_("Horizontal Space (%1$s)"),
				params_.length.asDocstring());
		break;
	case InsetSpaceParams::CUSTOM_PROTECTED:
		message = support::bformat(_("Protected Horizontal Space (%1$s)"),
				params_.length.asDocstring());
		break;
	}
	return message;
}


void InsetSpace::doDispatch(Cursor & cur, FuncRequest & cmd)
{
	switch (cmd.action) {

	case LFUN_INSET_MODIFY: {
		InsetSpaceParams params;
		InsetSpaceMailer::string2params(to_utf8(cmd.argument()), params);
		params_.kind = params.kind;
		params_.length = params.length;
		break;
	}

	case LFUN_MOUSE_RELEASE:
		if (!cur.selection() && cmd.button() == mouse_button::button1)
			InsetSpaceMailer(*this).showDialog(&cur.bv());
		break;

	default:
		Inset::doDispatch(cur, cmd);
		break;
	}
}


bool InsetSpace::getStatus(Cursor & cur, FuncRequest const & cmd,
	FuncStatus & status) const
{
	switch (cmd.action) {
	// we handle these
	case LFUN_INSET_MODIFY:
		if (cmd.getArg(0) == "space") {
			InsetSpaceParams params;
			InsetSpaceMailer::string2params(to_utf8(cmd.argument()), params);
			status.setOnOff(params_.kind == params.kind);
		} else
			status.enabled(true);
		return true;
	default:
		return Inset::getStatus(cur, cmd, status);
	}
}


void InsetSpace::edit(Cursor & cur, bool, EntryDirection)
{
	InsetSpaceMailer(*this).showDialog(&cur.bv());
}


void InsetSpace::metrics(MetricsInfo & mi, Dimension & dim) const
{
	if (isStretchableSpace()) {
		// The metrics for this kinds are calculated externally in
		// \c TextMetrics::computeRowMetrics. Those are dummy value:
		dim = Dimension(10, 10, 10);
		return;
	}

	frontend::FontMetrics const & fm = theFontMetrics(mi.base.font);
	dim.asc = fm.maxAscent();
	dim.des = fm.maxDescent();

	switch (params_.kind) {
		case InsetSpaceParams::THIN:
		case InsetSpaceParams::NEGTHIN:
			dim.wid = fm.width(char_type('M')) / 6;
			break;
		case InsetSpaceParams::PROTECTED:
		case InsetSpaceParams::NORMAL:
			dim.wid = fm.width(char_type(' '));
			break;
		case InsetSpaceParams::QUAD:
			dim.wid = fm.width(char_type('M'));
			break;
		case InsetSpaceParams::QQUAD:
			dim.wid = 2 * fm.width(char_type('M'));
			break;
		case InsetSpaceParams::ENSPACE:
		case InsetSpaceParams::ENSKIP:
			dim.wid = int(0.5 * fm.width(char_type('M')));
			break;
		case InsetSpaceParams::CUSTOM:
		case InsetSpaceParams::CUSTOM_PROTECTED:
			dim.wid = params_.length.inBP();
			break;
		case InsetSpaceParams::HFILL:
		case InsetSpaceParams::HFILL_PROTECTED:
		case InsetSpaceParams::DOTFILL:
		case InsetSpaceParams::HRULEFILL:
			// shut up compiler
			break;
	}
	// Cache the inset dimension.
	setDimCache(mi, dim);
}


void InsetSpace::draw(PainterInfo & pi, int x, int y) const
{
	Dimension const dim = dimension(*pi.base.bv);

	if (isStretchableSpace()) {
		int const asc = theFontMetrics(pi.base.font).ascent('M');
		int const desc = theFontMetrics(pi.base.font).descent('M');
		int const x0 = x + 1;
		int const x1 = x + dim.wid - 2;
		int const y0 = y + desc;
		int const y1 = y - asc;
		int const y2 = y - asc / 2;

		if (params_.kind == InsetSpaceParams::HFILL) {
			pi.pain.line(x0, y1, x0, y0, Color_added_space);
			pi.pain.line(x0, y2 , x1, y2, Color_added_space,
				frontend::Painter::line_onoffdash);
			pi.pain.line(x1, y1, x1, y0, Color_added_space);
		} else if (params_.kind == InsetSpaceParams::HFILL_PROTECTED) {
			pi.pain.line(x0, y1, x0, y0, Color_latex);
			pi.pain.line(x0, y2 , x1, y2, Color_latex,
				frontend::Painter::line_onoffdash);
			pi.pain.line(x1, y1, x1, y0, Color_latex);
		} else if (params_.kind == InsetSpaceParams::DOTFILL) {
			pi.pain.line(x0, y1, x0, y0, Color_special);
			pi.pain.line(x0, y, x1, y, Color_special,
				frontend::Painter::line_onoffdash);
			pi.pain.line(x1, y1, x1, y0, Color_special);
		} if (params_.kind == InsetSpaceParams::HRULEFILL) {
			pi.pain.line(x0, y1, x0, y0, Color_special);
			pi.pain.line(x0, y, x1, y, Color_special);
			pi.pain.line(x1, y1, x1, y0, Color_special);
		}
		return;
	}

	int const w = dim.wid;
	int const h = theFontMetrics(pi.base.font).ascent('x');
	int xp[4], yp[4];

	xp[0] = x;
	yp[0] = y - max(h / 4, 1);
	if (params_.kind == InsetSpaceParams::NORMAL ||
	    params_.kind == InsetSpaceParams::PROTECTED) {
		xp[1] = x;     yp[1] = y;
		xp[2] = x + w; yp[2] = y;
	} else {
		xp[1] = x;     yp[1] = y + max(h / 4, 1);
		xp[2] = x + w; yp[2] = y + max(h / 4, 1);
	}
	xp[3] = x + w;
	yp[3] = y - max(h / 4, 1);

	if (params_.kind == InsetSpaceParams::PROTECTED ||
	    params_.kind == InsetSpaceParams::ENSPACE ||
	    params_.kind == InsetSpaceParams::NEGTHIN ||
	    params_.kind == InsetSpaceParams::CUSTOM_PROTECTED)
		pi.pain.lines(xp, yp, 4, Color_latex);
	else
		pi.pain.lines(xp, yp, 4, Color_special);
}


void InsetSpaceParams::write(ostream & os) const
{
	string command;
	switch (kind) {
	case InsetSpaceParams::NORMAL:
		os << "\\space{}";
		break;
	case InsetSpaceParams::PROTECTED:
		os <<  "~";
		break;
	case InsetSpaceParams::THIN:
		os <<  "\\thinspace{}";
		break;
	case InsetSpaceParams::QUAD:
		os <<  "\\quad{}";
		break;
	case InsetSpaceParams::QQUAD:
		os <<  "\\qquad{}";
		break;
	case InsetSpaceParams::ENSPACE:
		os <<  "\\enspace{}";
		break;
	case InsetSpaceParams::ENSKIP:
		os <<  "\\enskip{}";
		break;
	case InsetSpaceParams::NEGTHIN:
		os <<  "\\negthinspace{}";
		break;
	case InsetSpaceParams::HFILL:
		os <<  "\\hfill{}";
		break;
	case InsetSpaceParams::HFILL_PROTECTED:
		os <<  "\\hspace*{\\fill}";
		break;
	case InsetSpaceParams::DOTFILL:
		os <<  "\\dotfill{}";
		break;
	case InsetSpaceParams::HRULEFILL:
		os <<  "\\hrulefill{}";
		break;
	case InsetSpaceParams::CUSTOM:
		os <<  "\\hspace{}";
		break;
	case InsetSpaceParams::CUSTOM_PROTECTED:
		os <<  "\\hspace*{}";
		break;
	}
	
	if (!length.empty())
		os << "\n\\length " << length.asString();
}


void InsetSpaceParams::read(Lexer & lex)
{
	lex.next();
	string const command = lex.getString();

	if (command == "\\space{}")
		kind = InsetSpaceParams::NORMAL;
	else if (command == "~")
		kind = InsetSpaceParams::PROTECTED;
	else if (command == "\\thinspace{}")
		kind = InsetSpaceParams::THIN;
	else if (command == "\\quad{}")
		kind = InsetSpaceParams::QUAD;
	else if (command == "\\qquad{}")
		kind = InsetSpaceParams::QQUAD;
	else if (command == "\\enspace{}")
		kind = InsetSpaceParams::ENSPACE;
	else if (command == "\\enskip{}")
		kind = InsetSpaceParams::ENSKIP;
	else if (command == "\\negthinspace{}")
		kind = InsetSpaceParams::NEGTHIN;
	else if (command == "\\hfill{}")
		kind = InsetSpaceParams::HFILL;
	else if (command == "\\hspace*{\\fill}")
		kind = InsetSpaceParams::HFILL_PROTECTED;
	else if (command == "\\dotfill{}")
		kind = InsetSpaceParams::DOTFILL;
	else if (command == "\\hrulefill{}")
		kind = InsetSpaceParams::HRULEFILL;
	else if (command == "\\hspace{}")
		kind = InsetSpaceParams::CUSTOM;
	else if (command == "\\hspace*{}")
		kind = InsetSpaceParams::CUSTOM_PROTECTED;
	else
		lex.printError("InsetSpace: Unknown kind: `$$Token'");


	string token;
	lex >> token;
	if (token == "\\length") {
		lex.next();
		string const len = lex.getString();
		length = Length(len);
		lex.next();
		token = lex.getString();
	}
	if (!lex)
		return;
	if (token != "\\end_inset")
		lex.printError("Missing \\end_inset at this point. "
			       "Read: `$$Token'");
}


void InsetSpace::write(ostream & os) const
{
	os << "Space ";
	params_.write(os);
}


void InsetSpace::read(Lexer & lex)
{
	params_.read(lex);
}


int InsetSpace::latex(odocstream & os, OutputParams const & runparams) const
{
	switch (params_.kind) {
	case InsetSpaceParams::NORMAL:
		os << (runparams.free_spacing ? " " : "\\ ");
		break;
	case InsetSpaceParams::PROTECTED:
		os << (runparams.free_spacing ? ' ' : '~');
		break;
	case InsetSpaceParams::THIN:
		os << (runparams.free_spacing ? " " : "\\,");
		break;
	case InsetSpaceParams::QUAD:
		os << (runparams.free_spacing ? " " : "\\quad{}");
		break;
	case InsetSpaceParams::QQUAD:
		os << (runparams.free_spacing ? " " : "\\qquad{}");
		break;
	case InsetSpaceParams::ENSPACE:
		os << (runparams.free_spacing ? " " : "\\enspace{}");
		break;
	case InsetSpaceParams::ENSKIP:
		os << (runparams.free_spacing ? " " : "\\enskip{}");
		break;
	case InsetSpaceParams::NEGTHIN:
		os << (runparams.free_spacing ? " " : "\\negthinspace{}");
		break;
	case InsetSpaceParams::HFILL:
		os << (runparams.free_spacing ? " " : "\\hfill{}");
		break;
	case InsetSpaceParams::HFILL_PROTECTED:
		os << (runparams.free_spacing ? " " : "\\hspace*{\\fill}");
		break;
	case InsetSpaceParams::DOTFILL:
		os << (runparams.free_spacing ? " " : "\\dotfill{}");
		break;
	case InsetSpaceParams::HRULEFILL:
		os << (runparams.free_spacing ? " " : "\\hrulefill{}");
		break;
	case InsetSpaceParams::CUSTOM:
		if (runparams.free_spacing)
			os << " ";
		else
			os << "\\hspace{" << from_ascii(params_.length.asLatexString()) << "}";
		break;
	case InsetSpaceParams::CUSTOM_PROTECTED:
		if (runparams.free_spacing)
			os << " ";
		else
			os << "\\hspace*{" << from_ascii(params_.length.asLatexString()) << "}";
		break;
	}
	return 0;
}


int InsetSpace::plaintext(odocstream & os, OutputParams const &) const
{
	switch (params_.kind) {
	case InsetSpaceParams::HFILL:
	case InsetSpaceParams::HFILL_PROTECTED:
		os << "     ";
		return 5;
	case InsetSpaceParams::DOTFILL:
		os << ".....";
		return 5;
	case InsetSpaceParams::HRULEFILL:
		os << "_____";
		return 5;
	default:
		os << ' ';
		return 1;
	}
}


int InsetSpace::docbook(odocstream & os, OutputParams const &) const
{
	switch (params_.kind) {
	case InsetSpaceParams::NORMAL:
	case InsetSpaceParams::QUAD:
	case InsetSpaceParams::QQUAD:
	case InsetSpaceParams::ENSKIP:
		os << " ";
		break;
	case InsetSpaceParams::PROTECTED:
	case InsetSpaceParams::ENSPACE:
	case InsetSpaceParams::THIN:
	case InsetSpaceParams::NEGTHIN:
		os << "&nbsp;";
		break;
	case InsetSpaceParams::HFILL:
	case InsetSpaceParams::HFILL_PROTECTED:
		os << '\n';
	case InsetSpaceParams::DOTFILL:
		// FIXME
		os << '\n';
	case InsetSpaceParams::HRULEFILL:
		// FIXME
		os << '\n';
	case InsetSpaceParams::CUSTOM:
	case InsetSpaceParams::CUSTOM_PROTECTED:
		// FIXME
		os << '\n';
	}
	return 0;
}


void InsetSpace::textString(odocstream & os) const
{
	plaintext(os, OutputParams(0));
}


bool InsetSpace::isStretchableSpace() const
{
	return (params_.kind == InsetSpaceParams::HFILL ||
		params_.kind == InsetSpaceParams::HFILL_PROTECTED ||
		params_.kind == InsetSpaceParams::DOTFILL ||
		params_.kind == InsetSpaceParams::HRULEFILL);
}


docstring InsetSpace::contextMenu(BufferView const &, int, int) const
{
	return from_ascii("context-space");
}


string const InsetSpaceMailer::name_ = "space";


InsetSpaceMailer::InsetSpaceMailer(InsetSpace & inset)
	: inset_(inset)
{}


string const InsetSpaceMailer::inset2string(Buffer const &) const
{
	return params2string(inset_.params());
}


void InsetSpaceMailer::string2params(string const & in, InsetSpaceParams & params)
{
	params = InsetSpaceParams();
	if (in.empty())
		return;

	istringstream data(in);
	Lexer lex(0,0);
	lex.setStream(data);

	string name;
	lex >> name;
	if (!lex || name != name_)
		return print_mailer_error("InsetSpaceMailer", in, 1, name_);

	params.read(lex);
}


string const InsetSpaceMailer::params2string(InsetSpaceParams const & params)
{
	ostringstream data;
	data << name_ << ' ';
	params.write(data);
	return data.str();
}


} // namespace lyx
