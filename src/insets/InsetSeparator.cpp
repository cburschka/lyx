/**
 * \file InsetSeparator.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Enrico Forestieri
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "InsetSeparator.h"

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
using namespace lyx::frontend;

namespace lyx {

InsetSeparator::InsetSeparator() : Inset(0)
{}


InsetSeparator::InsetSeparator(InsetSeparatorParams const & params)
	: Inset(0), params_(params)
{}


void InsetSeparatorParams::write(ostream & os) const
{
	switch (kind) {
	case InsetSeparatorParams::PLAIN:
		os <<  "plain";
		break;
	case InsetSeparatorParams::PARBREAK:
		os <<  "parbreak";
		break;
	case InsetSeparatorParams::LATEXPAR:
		os <<  "latexpar";
		break;
	}
}


void InsetSeparatorParams::read(Lexer & lex)
{
	string token;
	lex.setContext("InsetSeparatorParams::read");
	lex >> token;
	if (token == "plain")
		kind = InsetSeparatorParams::PLAIN;
	else if (token == "parbreak")
		kind = InsetSeparatorParams::PARBREAK;
	else if (token == "latexpar")
		kind = InsetSeparatorParams::LATEXPAR;
	else
		lex.printError("Unknown kind: `$$Token'");
}


void InsetSeparator::write(ostream & os) const
{
	os << "Separator ";
	params_.write(os);
}


void InsetSeparator::read(Lexer & lex)
{
	params_.read(lex);
	lex >> "\\end_inset";
}


void InsetSeparator::doDispatch(Cursor & cur, FuncRequest & cmd)
{
	switch (cmd.action()) {

	case LFUN_INSET_MODIFY: {
		InsetSeparatorParams params;
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


bool InsetSeparator::getStatus(Cursor & cur, FuncRequest const & cmd,
	FuncStatus & status) const
{
	switch (cmd.action()) {
	// we handle these
	case LFUN_INSET_MODIFY: {
		if (cmd.getArg(0) != "separator")
			break;
		InsetSeparatorParams params;
		string2params(to_utf8(cmd.argument()), params);
		status.setOnOff(params_.kind == params.kind);
		status.setEnabled(true);
		return true;
	}
	default:
		return Inset::getStatus(cur, cmd, status);
	}
	return false;
}


ColorCode InsetSeparator::ColorName() const
{
	return Color_latex;
}


void InsetSeparator::latex(otexstream & os, OutputParams const &) const
{
	// Do nothing if a paragraph break was just output
	if (!os.afterParbreak()) {
		switch (params_.kind) {
			case InsetSeparatorParams::PLAIN:
				os << breakln << "%\n";
				break;
			case InsetSeparatorParams::PARBREAK:
			case InsetSeparatorParams::LATEXPAR:
				os << breakln << "\n";
				break;
			default:
				os << breakln << "%\n";
				break;
		}
	}
}


int InsetSeparator::plaintext(odocstringstream & os,
        OutputParams const &, size_t) const
{
	os << '\n';
	return PLAINTEXT_NEWLINE;
}


int InsetSeparator::docbook(odocstream & os, OutputParams const &) const
{
	os << '\n';
	return 0;
}


docstring InsetSeparator::xhtml(XHTMLStream & xs, OutputParams const &) const
{
	xs << html::CR() << html::CompTag("br") << html::CR();
	return docstring();
}


void InsetSeparator::metrics(MetricsInfo & mi, Dimension & dim) const
{
	frontend::FontMetrics const & fm = theFontMetrics(mi.base.font);
	dim.asc = fm.maxAscent();
	dim.des = fm.maxDescent();
	dim.wid = fm.width('n');
	if (params_.kind != InsetSeparatorParams::LATEXPAR)
		dim.wid *= 8;
}


void InsetSeparator::draw(PainterInfo & pi, int x, int y) const
{
	FontInfo font;
	font.setColor(ColorName());

	frontend::FontMetrics const & fm = theFontMetrics(pi.base.font);
	int const wid = fm.width('n');
	int const asc = fm.maxAscent();

	int xp[7];
	int yp[7];

	if (params_.kind != InsetSeparatorParams::LATEXPAR) {
		yp[0] = int(y - 0.500 * asc * 0.75);
		yp[1] = yp[0];

		xp[0] = int(x);
		xp[1] = int(x + wid * 8);

		pi.pain.lines(xp, yp, 2, ColorName());

		if (params_.kind == InsetSeparatorParams::PARBREAK) {
			yp[0] += 0.25 * asc * 0.75;
			yp[1] = yp[0];
			pi.pain.lines(xp, yp, 2, ColorName());
		}
	} else {
		yp[0] = int(y - 0.500 * asc * 0.5);
		yp[1] = int(y - 0.250 * asc * 0.5);
		yp[2] = int(y);

		if (pi.ltr_pos) {
			xp[0] = int(x + 1 + wid * 0.375);
			xp[1] = int(x + 1);
		} else {
			xp[0] = int(x - 1 + wid * 0.625);
			xp[1] = int(x - 1 + wid);
		}
		xp[2] = xp[0];

		pi.pain.lines(xp, yp, 3, ColorName(), Painter::fill_oddeven);

		yp[0] = yp[1];
		yp[2] = int(y - 0.850 * asc * 0.5);
		yp[3] = int(y - 1.250 * asc * 0.5);
		yp[4] = yp[3];
		yp[5] = yp[2];
		yp[6] = yp[5];

		xp[0] = xp[1];
		if (pi.ltr_pos) {
			xp[1] = int(x + 1 + wid * 0.50);
			xp[2] = int(x + wid);
			xp[3] = xp[2];
			xp[4] = int(x + wid * 0.75);
		} else {
			xp[1] = int(x + wid * 0.50);
			xp[2] = int(x);
			xp[3] = xp[2];
			xp[4] = int(x + wid * 0.25);
		}
		xp[5] = xp[4];
		xp[6] = xp[2];

		int c1x[7];
		int c1y[7];
		int c2x[7];
		int c2y[7];

		for (int i = 1; i < 7; ++i) {
			c1x[i] = xp[i - 1];
			c1y[i] = yp[i - 1];
			c2x[i] = xp[i];
			c2y[i] = yp[i];
		}

		int d = pi.ltr_pos ? yp[4] - yp[5] : yp[5] - yp[4];

		c1x[2] = xp[2];
		c2y[2] = int(y - 0.500 * asc * 0.5);
		c1x[5] += d;
		c2x[5] += d;

		pi.pain.path(xp, yp, c1x, c1y, c2x, c2y, 7, ColorName());
	}
}


string InsetSeparator::contextMenuName() const
{
	if (params_.kind == InsetSeparatorParams::LATEXPAR)
		return string();

	return "context-separator";
}


void InsetSeparator::string2params(string const & in, InsetSeparatorParams & params)
{
	params = InsetSeparatorParams();
	if (in.empty())
		return;
	istringstream data(in);
	Lexer lex;
	lex.setStream(data);
	lex.setContext("InsetSeparator::string2params");
	lex >> "separator";
	params.read(lex);
}


string InsetSeparator::params2string(InsetSeparatorParams const & params)
{
	ostringstream data;
	data << "separator" << ' ';
	params.write(data);
	return data.str();
}


} // namespace lyx
