/**
 * \file insetnewline.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "insetnewline.h"

#include "BufferView.h"
#include "debug.h"
#include "lyxtext.h"
#include "metricsinfo.h"
#include "paragraph_funcs.h"

#include "frontends/font_metrics.h"
#include "frontends/Painter.h"

using std::endl;
using std::ostream;


void InsetNewline::read(Buffer const &, LyXLex &)
{
	/* Nothing to read */
}


void InsetNewline::write(Buffer const &, ostream & os) const
{
	os << "\n\\newline \n";
}


void InsetNewline::metrics(MetricsInfo & mi, Dimension & dim) const
{
	LyXFont & font = mi.base.font;
	dim.asc = font_metrics::maxAscent(font);
	dim.des = font_metrics::maxDescent(font);
	dim.wid = font_metrics::width('n', font);
	dim_ = dim;
}


int InsetNewline::latex(Buffer const &, ostream &,
			LatexRunParams const &) const
{
	lyxerr << "Eek, calling InsetNewline::latex !" << endl;
	return 0;
}


int InsetNewline::ascii(Buffer const &, ostream & os, int) const
{
	os << '\n';
	return 0;
}


int InsetNewline::linuxdoc(Buffer const &, std::ostream & os) const
{
	os << '\n';
	return 0;
}


int InsetNewline::docbook(Buffer const &, std::ostream & os, bool) const
{
	os << '\n';
	return 0;
}


void InsetNewline::draw(PainterInfo & pi, int x, int y) const
{
	int const wid = font_metrics::width('n', pi.base.font);
	int const asc = font_metrics::maxAscent(pi.base.font);

	// hack, and highly dubious
	lyx::pos_type pos = ownerPar(*pi.base.bv->buffer(), this)
		.getPositionOfInset(this);
	bool const ltr_pos = (pi.base.bv->text->bidi_level(pos) % 2 == 0);

	int xp[3];
	int yp[3];

	yp[0] = int(y - 0.875 * asc * 0.75);
	yp[1] = int(y - 0.500 * asc * 0.75);
	yp[2] = int(y - 0.125 * asc * 0.75);

	if (ltr_pos) {
		xp[0] = int(x + wid * 0.375);
		xp[1] = int(x);
		xp[2] = int(x + wid * 0.375);
	} else {
		xp[0] = int(x + wid * 0.625);
		xp[1] = int(x + wid);
		xp[2] = int(x + wid * 0.625);
	}

	pi.pain.lines(xp, yp, 3, LColor::eolmarker);

	yp[0] = int(y - 0.500 * asc * 0.75);
	yp[1] = int(y - 0.500 * asc * 0.75);
	yp[2] = int(y - asc * 0.75);

	if (ltr_pos) {
		xp[0] = int(x);
		xp[1] = int(x + wid);
		xp[2] = int(x + wid);
	} else {
		xp[0] = int(x + wid);
		xp[1] = int(x);
		xp[2] = int(x);
	}

	pi.pain.lines(xp, yp, 3, LColor::eolmarker);
}
