/**
 * \file Painter.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author unknown
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "Painter.h"
#include "font_metrics.h"
#include "WorkArea.h"

#include "LColor.h"
#include "lyxfont.h"

using std::max;
using std::string;


void Painter::button(int x, int y, int w, int h)
{
	fillRectangle(x, y, w, h, LColor::buttonbg);
	buttonFrame(x, y, w, h);
}


void Painter::buttonFrame(int x, int y, int w, int h)
{
	//  Width of a side of the button
	int const d = 2;

	fillRectangle(x, y, w, d, LColor::top);
	fillRectangle(x, (y + h - d), w, d, LColor::bottom);

	// Now a couple of trapezoids
	int x1[4], y1[4];

	x1[0] = x + d;   y1[0] = y + d;
	x1[1] = x + d;   y1[1] = y + h - d;
	x1[2] = x;       y1[2] = y + h;
	x1[3] = x;       y1[3] = y;
	fillPolygon(x1, y1, 4, LColor::left);

	x1[0] = x + w - d;  y1[0] = y + d;
	x1[1] = x + w - d;  y1[1] = y + h - d;
	x1[2] = x + w;      y1[2] = y + h - d;
	x1[3] = x + w;      y1[3] = y;
	fillPolygon(x1, y1, 4, LColor::right);
}


void Painter::rectText(int x, int y,
	string const & str,
	LyXFont const & font,
	LColor_color back,
	LColor_color frame)
{
	int width;
	int ascent;
	int descent;

	font_metrics::rectText(str, font, width, ascent, descent);

	if (back != LColor::none)
		fillRectangle(x + 1, y - ascent + 1, width - 1,
			      ascent + descent - 1, back);

	if (frame != LColor::none)
		rectangle(x, y - ascent, width, ascent + descent, frame);

	text(x + 3, y, str, font);
}


void Painter::buttonText(int x, int y, string const & str, LyXFont const & font)
{
	int width;
	int ascent;
	int descent;

	font_metrics::buttonText(str, font, width, ascent, descent);

	button(x, y - ascent, width, descent + ascent);
	text(x + 4, y, str, font);
}


void Painter::underline(LyXFont const & f, int x, int y, int width)
{
	int const below = max(font_metrics::maxDescent(f) / 2, 2);
	int const height = max((font_metrics::maxDescent(f) / 4) - 1, 1);

	if (height < 2)
		line(x, y + below, x + width, y + below, f.color());
	else
		fillRectangle(x, y + below, width, below + height, f.color());
}
