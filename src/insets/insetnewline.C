/**
 * \file insetnewline.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS
 */

#include <config.h>


#include "BufferView.h"
#include "paragraph.h"
#include "lyxtext.h"
#include "insetnewline.h"
#include "support/LOstream.h"
#include "frontends/Painter.h"
#include "frontends/font_metrics.h"

#include "debug.h"

using std::ostream;
using std::endl;

InsetNewline::InsetNewline()
	: Inset()
{}


void InsetNewline::read(Buffer const *, LyXLex &)
{
	/* Nothing to read */
}


void InsetNewline::write(Buffer const *, ostream & os) const
{
	os << "\n\\newline \n";
}


int InsetNewline::ascent(BufferView *, LyXFont const & font) const
{
	return font_metrics::maxAscent(font);
}


int InsetNewline::descent(BufferView *, LyXFont const & font) const
{
	return font_metrics::maxDescent(font);
}


int InsetNewline::width(BufferView *, LyXFont const & font) const
{
	return font_metrics::width('n', font);
}


int InsetNewline::latex(Buffer const *, ostream &, bool, bool) const
{
	lyxerr << "Eek, calling InsetNewline::latex !" << endl;
	return 0;
}


int InsetNewline::ascii(Buffer const *, ostream & os, int) const
{
	os << '\n';
	return 0;
}


int InsetNewline::linuxdoc(Buffer const *, std::ostream &) const
{
	/* FIXME */
	return 0;
}


int InsetNewline::docbook(Buffer const *, std::ostream &, bool) const
{
	/* FIXME */
	return 0;
}


void InsetNewline::draw(BufferView * bv, LyXFont const & font,
	                int baseline, float & x) const
{
	Painter & pain(bv->painter());

	int const wid = font_metrics::width('n', font);
	int const asc = font_metrics::maxAscent(font);
	int const y = baseline;

	// hack, and highly dubious
	lyx::pos_type pos = parOwner()->getPositionOfInset(this);
	bool const ltr_pos = (bv->text->bidi_level(pos) % 2 == 0);

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

	pain.lines(xp, yp, 3, LColor::eolmarker);

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

	pain.lines(xp, yp, 3, LColor::eolmarker);

	x += wid;
}
