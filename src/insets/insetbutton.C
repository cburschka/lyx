/* This file is part of
 * ======================================================
 *
 *           LyX, The Document Processor
 *
 *	    Copyright 1995 Matthias Ettrich
 *          Copyright 2000-2001 The LyX Team.
 *
 * ====================================================== */

#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include "insetbutton.h"
#include "debug.h"
#include "BufferView.h"
#include "frontends/Painter.h"
#include "support/LAssert.h"
#include "lyxfont.h"
#include "frontends/font_metrics.h"

using std::ostream;
using std::endl;


int InsetButton::ascent(BufferView * bv, LyXFont const &) const
{
	lyx::Assert(bv);

	LyXFont font(LyXFont::ALL_SANE);
	font.decSize();

	int width;
	int ascent;
	int descent;
	string const s = getScreenLabel(bv->buffer());

        if (editable()) {
		font_metrics::buttonText(s, font, width, ascent, descent);
	} else {
		font_metrics::rectText(s, font, width, ascent, descent);
	}

	return ascent;
}


int InsetButton::descent(BufferView * bv, LyXFont const &) const
{
	lyx::Assert(bv);

	LyXFont font(LyXFont::ALL_SANE);
	font.decSize();

	int width;
	int ascent;
	int descent;
	string const s = getScreenLabel(bv->buffer());

        if (editable()) {
		font_metrics::buttonText(s, font, width, ascent, descent);
	} else {
		font_metrics::rectText(s, font, width, ascent, descent);
	}

	return descent;
}


int InsetButton::width(BufferView * bv, LyXFont const &) const
{
	lyx::Assert(bv);

	LyXFont font(LyXFont::ALL_SANE);
	font.decSize();

	int width;
	int ascent;
	int descent;
	string const s = getScreenLabel(bv->buffer());

        if (editable()) {
		font_metrics::buttonText(s, font, width, ascent, descent);
	} else {
		font_metrics::rectText(s, font, width, ascent, descent);
	}

	return width + 4;
}


void InsetButton::draw(BufferView * bv, LyXFont const &,
			int baseline, float & x, bool) const
{
	lyx::Assert(bv);

	Painter & pain = bv->painter();
	// Draw it as a box with the LaTeX text
	LyXFont font(LyXFont::ALL_SANE);
	font.setColor(LColor::command).decSize();

	string const s = getScreenLabel(bv->buffer());

	if (editable()) {
		pain.buttonText(int(x) + 2, baseline, s, font);
	} else {
		pain.rectText(int(x) + 2, baseline, s, font,
			      LColor::commandbg, LColor::commandframe);
	}

	x += width(bv, font);
}
