/**
 * \file InsetNewpage.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author André Pönitz
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "InsetNewpage.h"

#include "Text.h"
#include "MetricsInfo.h"
#include "OutputParams.h"
#include "TextMetrics.h"

#include "frontends/FontMetrics.h"
#include "frontends/Painter.h"

#include "support/debug.h"
#include "support/docstring.h"
#include "support/docstream.h"
#include "support/gettext.h"

using namespace std;

namespace lyx {

void InsetNewpage::read( Lexer &)
{
	/* Nothing to read */
}


void InsetNewpage::write(ostream & os) const
{
	os << "\n" << getCmdName() << '\n';
}


void InsetNewpage::metrics(MetricsInfo & mi, Dimension & dim) const
{
	dim.asc = defaultRowHeight();
	dim.des = defaultRowHeight();
	dim.wid = mi.base.textwidth;
	// Cache the inset dimension. 
	setDimCache(mi, dim);
}


void InsetNewpage::draw(PainterInfo & pi, int x, int y) const
{
	using frontend::Painter;

	FontInfo font;
	font.setColor(ColorName());
	font.decSize();

	Dimension const dim = dimension(*pi.base.bv);

	int w = 0;
	int a = 0;
	int d = 0;
	theFontMetrics(font).rectText(insetLabel(), w, a, d);

	int const text_start = int(x + (dim.wid - w) / 2);
	int const text_end = text_start + w;

	pi.pain.rectText(text_start, y + d, insetLabel(), font,
		Color_none, Color_none);

	pi.pain.line(x, y, text_start, y,
		   ColorName(), Painter::line_onoffdash);
	pi.pain.line(text_end, y, int(x + dim.wid), y,
		   ColorName(), Painter::line_onoffdash);
}


int InsetNewpage::latex(odocstream & os, OutputParams const &) const
{
	os << from_ascii(getCmdName()) << "{}";
	return 0;
}


int InsetNewpage::plaintext(odocstream & os, OutputParams const &) const
{
	os << '\n';
	return PLAINTEXT_NEWLINE;
}


int InsetNewpage::docbook(odocstream & os, OutputParams const &) const
{
	os << '\n';
	return 0;
}


} // namespace lyx
