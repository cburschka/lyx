/* This file is part of
 * ======================================================
 * 
 *           LyX, The Document Processor
 * 	 
 *	    Copyright 1998-2000 The LyX Team
 *
 *======================================================*/

#include <config.h>

#ifdef USE_PAINTER

#ifdef __GNUG__
#pragma implementation
#endif

#include "PainterBase.h"
#include "lyxfont.h"
#include "WorkArea.h"


int PainterBase::dummy1 = 0;
int PainterBase::dummy2 = 0;
int PainterBase::dummy3 = 0;

int PainterBase::paperMargin()
{
	return 20;
}


int PainterBase::paperWidth()
{
	return owner.workWidth();
}


int PainterBase::width(string const & s, LyXFont const & f)
{
	return f.stringWidth(s);
}


int PainterBase::width(char const * s, int l, LyXFont const & f)
{
	return f.textWidth(s, l);
}


int PainterBase::width(char c, LyXFont const & f)
{
	return f.width(c);
}


PainterBase & PainterBase::circle(int x, int y, unsigned int d,
				  LColor::color col = LColor::foreground)
{
	return ellipse(x, y, d, d, col);
}


PainterBase & PainterBase::ellipse(int x, int y,
				   unsigned int w, unsigned int h,
				   LColor::color col = LColor::foreground)
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
				    LColor::color frame, bool draw,
				    int & width, int & ascent, int & descent)
{
	static int const d = 2;
	width = this->width(str, font) + d * 2 + 2;
	ascent = font.maxAscent() + d;
	descent = font.maxDescent() + d;

	if (!draw) return *this;

	rectangle(x, baseline - ascent, width, ascent + descent, frame);
	fillRectangle(x + 1, baseline - ascent + 1, width - 1, 
		      ascent + descent - 1, back);
	text(x + d, baseline, str, font);
	return *this;
}


PainterBase & PainterBase::buttonText(int x, int baseline,
				      string const & str, 
				      LyXFont const & font, bool draw,
				      int & width, int & ascent, int & descent)
{
	width = this->width(str, font) + 8;
	ascent = font.maxAscent() + 3;
	descent = font.maxDescent() + 3;

	if (!draw) return *this;

	button(x, baseline - ascent, width, descent + ascent);
	text(x + 4, baseline, str, font);
	return *this;
}

#endif
