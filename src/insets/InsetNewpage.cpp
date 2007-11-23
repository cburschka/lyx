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

#include "debug.h"
#include "gettext.h"
#include "Text.h"
#include "MetricsInfo.h"
#include "OutputParams.h"
#include "TextMetrics.h"

#include "frontends/FontMetrics.h"
#include "frontends/Painter.h"

#include "support/docstring.h"


namespace lyx {

using frontend::Painter;


void InsetNewpage::read(Buffer const &, Lexer &)
{
	/* Nothing to read */
}


void InsetNewpage::write(Buffer const &, std::ostream & os) const
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
	FontInfo font;
	font.setColor(Color_pagebreak);
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
		   Color_pagebreak, Painter::line_onoffdash);
	pi.pain.line(text_end, y, int(x + dim.wid), y,
		   Color_pagebreak, Painter::line_onoffdash);
}


int InsetNewpage::latex(Buffer const &, odocstream & os,
			  OutputParams const &) const
{
	os << from_ascii(getCmdName()) << "{}";
	return 0;
}


int InsetNewpage::plaintext(Buffer const &, odocstream & os,
			      OutputParams const &) const
{
	os << '\n';
	return PLAINTEXT_NEWLINE;
}


int InsetNewpage::docbook(Buffer const &, odocstream & os,
			    OutputParams const &) const
{
	os << '\n';
	return 0;
}


} // namespace lyx
