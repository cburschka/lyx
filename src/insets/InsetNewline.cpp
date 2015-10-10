/**
 * \file InsetNewline.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 * \author Jürgen Spitzmüller
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "InsetNewline.h"

#include "Cursor.h"
#include "Dimension.h"
#include "FuncRequest.h"
#include "FuncStatus.h"
#include "Lexer.h"
#include "MetricsInfo.h"
#include "OutputParams.h"
#include "output_xhtml.h"

#include "frontends/Application.h"
#include "frontends/FontMetrics.h"
#include "frontends/Painter.h"

#include "support/debug.h"
#include "support/docstream.h"
#include "support/docstring.h"

using namespace std;

namespace lyx {

InsetNewline::InsetNewline() : Inset(0)
{}


void InsetNewlineParams::write(ostream & os) const
{
	switch (kind) {
	case InsetNewlineParams::NEWLINE:
		os << "newline";
		break;
	case InsetNewlineParams::LINEBREAK:
		os <<  "linebreak";
		break;
	}
}


void InsetNewlineParams::read(Lexer & lex)
{
	string token;
	lex.setContext("InsetNewlineParams::read");
	lex >> token;	
	if (token == "newline")
		kind = InsetNewlineParams::NEWLINE;
	else if (token == "linebreak")
		kind = InsetNewlineParams::LINEBREAK;
	else
		lex.printError("Unknown kind: `$$Token'");
}


void InsetNewline::write(ostream & os) const
{
	os << "Newline ";
	params_.write(os);
}


void InsetNewline::read(Lexer & lex)
{
	params_.read(lex);
	lex >> "\\end_inset";
}


void InsetNewline::metrics(MetricsInfo & mi, Dimension & dim) const
{
	frontend::FontMetrics const & fm = theFontMetrics(mi.base.font);
	dim.asc = fm.maxAscent();
	dim.des = fm.maxDescent();
	dim.wid = fm.width('n');
}


void InsetNewline::doDispatch(Cursor & cur, FuncRequest & cmd)
{
	switch (cmd.action()) {

	case LFUN_INSET_MODIFY: {
		InsetNewlineParams params;
		cur.recordUndo();
		string2params(to_utf8(cmd.argument()), params);
		params_.kind = params.kind;
		break;
	}

	default:
		Inset::doDispatch(cur, cmd);
		break;
	}
}


bool InsetNewline::getStatus(Cursor & cur, FuncRequest const & cmd,
	FuncStatus & status) const
{
	switch (cmd.action()) {
	// we handle these
	case LFUN_INSET_MODIFY:
		if (cmd.getArg(0) == "newline") {
			InsetNewlineParams params;
			string2params(to_utf8(cmd.argument()), params);
			status.setOnOff(params_.kind == params.kind);
		}
		status.setEnabled(true);
		return true;
	default:
		return Inset::getStatus(cur, cmd, status);
	}
}


ColorCode InsetNewline::ColorName() const
{
	switch (params_.kind) {
		case InsetNewlineParams::NEWLINE:
			return Color_eolmarker;
			break;
		case InsetNewlineParams::LINEBREAK:
			return Color_pagebreak;
			break;
	}
	// not really useful, but to avoids gcc complaints
	return Color_eolmarker;
}


void InsetNewline::latex(otexstream & os, OutputParams const & rp) const
{
	switch (params_.kind) {
		case InsetNewlineParams::NEWLINE:
			if (rp.inTableCell == OutputParams::PLAIN)
				os << "\\newline\n";
			else
				os << "\\\\\n";
			break;
		case InsetNewlineParams::LINEBREAK:
			os << "\\linebreak{}\n";
			break;
		default:
			os << "\\\\\n";
			break;
	}
}


int InsetNewline::plaintext(odocstringstream & os,
        OutputParams const &, size_t) const
{
	os << '\n';
	return PLAINTEXT_NEWLINE;
}


int InsetNewline::docbook(odocstream & os, OutputParams const &) const
{
	os << '\n';
	return 0;
}


docstring InsetNewline::xhtml(XHTMLStream & xs, OutputParams const &) const
{
	xs << html::CR() << html::CompTag("br") << html::CR();
	return docstring();
}


void InsetNewline::draw(PainterInfo & pi, int x, int y) const
{
	FontInfo font;
	font.setColor(ColorName());

	frontend::FontMetrics const & fm = theFontMetrics(pi.base.font);
	int const wid = fm.width('n');
	int const asc = fm.maxAscent();

	int xp[3];
	int yp[3];

	yp[0] = int(y - 0.875 * asc * 0.75);
	yp[1] = int(y - 0.500 * asc * 0.75);
	yp[2] = int(y - 0.125 * asc * 0.75);

	if (pi.ltr_pos) {
		xp[0] = int(x + wid * 0.375);
		xp[1] = int(x);
		xp[2] = int(x + wid * 0.375);
	} else {
		xp[0] = int(x + wid * 0.625);
		xp[1] = int(x + wid);
		xp[2] = int(x + wid * 0.625);
	}

	pi.pain.lines(xp, yp, 3, ColorName());

	yp[0] = int(y - 0.500 * asc * 0.75);
	yp[1] = int(y - 0.500 * asc * 0.75);
	yp[2] = int(y - asc * 0.75);

	if (pi.ltr_pos) {
		xp[0] = int(x);
		xp[1] = int(x + wid);
		xp[2] = int(x + wid);
	} else {
		xp[0] = int(x + wid);
		xp[1] = int(x);
		xp[2] = int(x);
	}

	pi.pain.lines(xp, yp, 3, ColorName());

	if (params_.kind == InsetNewlineParams::LINEBREAK) {

		yp[2] = int(y - 0.500 * asc * 0.75);

		if (pi.ltr_pos) {
			xp[0] = int(x + 1.3 * wid);
			xp[1] = int(x + 2 * wid);
			xp[2] = int(x + 2 * wid);
		} else {
			xp[0] = int(x - 0.3 * wid);
			xp[1] = int(x - wid);
			xp[2] = int(x - wid);
		}
		pi.pain.lines(xp, yp, 3, ColorName());

		yp[0] = int(y - 0.875 * asc * 0.75);
		yp[1] = int(y - 0.500 * asc * 0.75);
		yp[2] = int(y - 0.125 * asc * 0.75);
	
		if (pi.ltr_pos) {
			xp[0] = int(x + 2 * wid * 0.813);
			xp[1] = int(x + 2 * wid);
			xp[2] = int(x + 2 * wid * 0.813);
		} else {
			xp[0] = int(x - wid * 0.625);
			xp[1] = int(x - wid);
			xp[2] = int(x - wid * 0.625);
		}
		pi.pain.lines(xp, yp, 3, ColorName());
	}
}


string InsetNewline::contextMenuName() const
{
	return "context-newline";
}


void InsetNewline::string2params(string const & in, InsetNewlineParams & params)
{
	params = InsetNewlineParams();
	if (in.empty())
		return;
	istringstream data(in);
	Lexer lex;
	lex.setStream(data);
	lex.setContext("InsetNewline::string2params");
	lex >> "newline";
	params.read(lex);
}


string InsetNewline::params2string(InsetNewlineParams const & params)
{
	ostringstream data;
	data << "newline" << ' ';
	params.write(data);
	return data.str();
}


} // namespace lyx
