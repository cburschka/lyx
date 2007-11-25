/**
 * \file InsetNewline.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "InsetNewline.h"

#include "debug.h"
#include "Dimension.h"
#include "MetricsInfo.h"
#include "OutputParams.h"

#include "frontends/FontMetrics.h"
#include "frontends/Painter.h"

#include "support/docstring.h"


namespace lyx {

using std::endl;
using std::ostream;


void InsetNewline::read(Buffer const &, Lexer &)
{
	/* Nothing to read */
}


void InsetNewline::write(Buffer const &, ostream & os) const
{
	os << "\n" << getLyXName() << '\n';
}


void InsetNewline::metrics(MetricsInfo & mi, Dimension & dim) const
{
	frontend::FontMetrics const & fm = theFontMetrics(mi.base.font);
	dim.asc = fm.maxAscent();
	dim.des = fm.maxDescent();
	dim.wid = fm.width('n');
}


int InsetNewline::latex(Buffer const &, odocstream & os,
			OutputParams const &) const
{
	os << from_ascii(getCmdName()) << '\n';
	return 0;
}


int InsetNewline::plaintext(Buffer const &, odocstream & os,
			    OutputParams const &) const
{
	os << '\n';
	return PLAINTEXT_NEWLINE;
}


int InsetNewline::docbook(Buffer const &, odocstream & os,
			  OutputParams const &) const
{
	os << '\n';
	return 0;
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

	// add label text behind the newline marker to divide from \newline
	int w = 0;
	int a = 0;
	int d = 0;
	theFontMetrics(font).rectText(insetLabel(), w, a, d);
	
	int const text_start = int(x + 2 * wid);
			
	pi.pain.rectText(text_start, yp[0] + d, insetLabel(), font,
		Color_none, Color_none);
}


bool InsetNewline::isSpace() const
{
	return true;
}


} // namespace lyx
