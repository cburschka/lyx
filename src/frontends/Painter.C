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

#include "frontends/Painter.h"

#include "frontends/Application.h"
#include "frontends/FontLoader.h"
#include "frontends/FontMetrics.h"

#include "LColor.h"
#include "lyxfont.h"

using lyx::docstring;

using std::max;
using std::string;

namespace lyx {
namespace frontend {

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
	fillRectangle(x, y + h - d, w, d, LColor::bottom);

	for (int i = 0 ; i < d ; ++i) {
		line(x + i, y + i,
		     x + i, y + h - 1 - i, LColor::left);
		line(x + w - 1 - i, y + i + 1,
		     x + w - 1 - i, y + h - 1 - i, LColor::right);
	}
}


void Painter::rectText(int x, int y,
	docstring const & str,
	LyXFont const & font,
	LColor_color back,
	LColor_color frame)
{
	int width;
	int ascent;
	int descent;

	FontMetrics const & fm = theApp->fontLoader().metrics(font);
	fm.rectText(str, width, ascent, descent);

	if (back != LColor::none)
		fillRectangle(x + 1, y - ascent + 1, width - 1,
			      ascent + descent - 1, back);

	if (frame != LColor::none)
		rectangle(x, y - ascent, width, ascent + descent, frame);

	text(x + 3, y, str, font);
}


void Painter::buttonText(int x, int y, docstring const & str, LyXFont const & font)
{
	int width;
	int ascent;
	int descent;

	FontMetrics const & fm = theApp->fontLoader().metrics(font);
	fm.buttonText(str, width, ascent, descent);

	button(x, y - ascent, width, descent + ascent);
	text(x + 4, y, str, font);
}


void Painter::underline(LyXFont const & f, int x, int y, int width)
{
	FontMetrics const & fm = theApp->fontLoader().metrics(f);

	int const below = max(fm.maxDescent() / 2, 2);
	int const height = max((fm.maxDescent() / 4) - 1, 1);

	if (height < 2)
		line(x, y + below, x + width, y + below, f.color());
	else
		fillRectangle(x, y + below, width, below + height, f.color());
}

} // namespace frontend
} // namespace lyx
