/* This file is part of
 * ======================================================
 * 
 *           LyX, The Document Processor
 * 	 
 *	    Copyright 1998-2001 The LyX Team
 *
 *======================================================*/

#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include "PainterBase.h"
#include "lyxfont.h"
#include "WorkArea.h"
#include "font.h"

int PainterBase::dummy1 = 0;
int PainterBase::dummy2 = 0;
int PainterBase::dummy3 = 0;

int PainterBase::paperMargin() const
{
	return 20;
}


int PainterBase::paperWidth() const
{
	return owner.workWidth();
}


int PainterBase::paperHeight() const
{
	return owner.height();
}


PainterBase & PainterBase::circle(int x, int y, unsigned int d,
				  LColor::color col)
{
	return ellipse(x, y, d, d, col);
}


PainterBase & PainterBase::ellipse(int x, int y,
				   unsigned int w, unsigned int h,
				   LColor::color col)
{
	return arc(x, y, w, h, 0, 0, col);
}


PainterBase & PainterBase::button(int x, int y, int w, int h)
{
	fillRectangle(x, y, w, h, LColor::buttonbg);
	buttonFrame(x, y, w, h);
	return * this;
}


PainterBase & PainterBase::buttonFrame(int x, int y, int w, int h)
{
	//  Width of a side of the button
	int d = 2;

	fillRectangle(x, y, w, d, LColor::top);
	fillRectangle(x, (y+h-d), w, d, LColor::bottom);
 
	// Now a couple of trapezoids
	int x1[4], y1[4];
 
	x1[0] = x+d;   y1[0] = y+d;
	x1[1] = x+d;   y1[1] = (y+h-d);
	x1[2] = x;     y1[2] = y+h;
	x1[3] = x;     y1[3] = y;
	fillPolygon(x1, y1, 4, LColor::left);

	x1[0] = (x+w-d); y1[0] = y+d;
	x1[1] = (x+w-d); y1[1] = (y+h-d);
	x1[2] = x+w; y1[2] = (y+h-d);
	x1[3] = x+w; y1[3] = y;
	fillPolygon(x1, y1, 4, LColor::right);

	return *this;
}


PainterBase & PainterBase::rectText(int x, int baseline, 
				    string const & str, 
				    LyXFont const & font,
				    LColor::color back,
				    LColor::color frame)
{
	int width;
	int ascent;
	int descent;
	
	lyxfont::rectText(str, font, width, ascent, descent);
	rectangle(x, baseline - ascent, width, ascent + descent, frame);
	fillRectangle(x + 1, baseline - ascent + 1, width - 1, 
		      ascent + descent - 1, back);
	text(x + 3, baseline, str, font);
	return *this;
}


PainterBase & PainterBase::buttonText(int x, int baseline,
				      string const & str, 
				      LyXFont const & font)
{
	int width;
	int ascent;
	int descent;
	
	lyxfont::buttonText(str, font, width, ascent, descent);
	button(x, baseline - ascent, width, descent + ascent);
	text(x + 4, baseline, str, font);
	return *this;
}
