/* This file is part of
 * ======================================================
 * 
 *           LyX, The Document Processor
 * 	 
 *	    Copyright 1995 Matthias Ettrich
 *          Copyright 2000 The LyX Team.
 *
 * ====================================================== */

#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include "insetbutton.h"
#include "debug.h"
#include "BufferView.h"
#include "Painter.h"

using std::ostream;
using std::endl;


int InsetButton::ascent(BufferView * bv, LyXFont const &) const
{
	LyXFont font(LyXFont::ALL_SANE);
	font.decSize();
	
	int width, ascent, descent;
        string s = getScreenLabel();
	
        if (Editable()) {
		bv->painter().buttonText(0, 0, s, font,
					 false, width, ascent, descent);
	} else {
		bv->painter().rectText(0, 0, s, font,
				       LColor::commandbg, LColor::commandframe,
				       false, width, ascent, descent);
	}
	return ascent;
}


int InsetButton::descent(BufferView * bv, LyXFont const &) const
{
	LyXFont font(LyXFont::ALL_SANE);
	font.decSize();
	
	int width, ascent, descent;
        string s = getScreenLabel();
	
        if (Editable()) {
		bv->painter().buttonText(0, 0, s, font,
					 false, width, ascent, descent);
	} else {
		bv->painter().rectText(0, 0, s, font,
				   LColor::commandbg, LColor::commandframe,
				   false, width, ascent, descent);
	}
	return descent;
}


int InsetButton::width(BufferView * bv, LyXFont const &) const
{
	LyXFont font(LyXFont::ALL_SANE);
	font.decSize();
	
	int width, ascent, descent;
        string s = getScreenLabel();
	
        if (Editable()) {
		bv->painter().buttonText(0, 0, s, font,
					 false, width, ascent, descent);
	} else {
		bv->painter().rectText(0, 0, s, font,
				       LColor::commandbg, LColor::commandframe,
				       false, width, ascent, descent);
	}
	return width + 4;
}


void InsetButton::draw(BufferView * bv, LyXFont const &,
			int baseline, float & x, bool) const
{
	Painter & pain = bv->painter();
	// Draw it as a box with the LaTeX text
	LyXFont font(LyXFont::ALL_SANE);
	font.setColor(LColor::command).decSize();

	int width;
	string s = getScreenLabel();

	if (Editable()) {
		pain.buttonText(int(x)+2, baseline, s, font, true, width);
	} else {
		pain.rectText(int(x)+2, baseline, s, font,
			      LColor::commandbg, LColor::commandframe,
			      true, width);
	}

	x += width + 4;
}
