/**
 * \file insetpagebreak.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author André Pönitz
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "insetpagebreak.h"

#include "debug.h"
#include "gettext.h"
#include "LColor.h"
#include "lyxtext.h"
#include "metricsinfo.h"
#include "outputparams.h"
#include "TextMetrics.h"

#include "frontends/FontMetrics.h"
#include "frontends/Painter.h"


namespace lyx {

using frontend::Painter;

using std::endl;
using std::ostream;


void InsetPagebreak::read(Buffer const &, LyXLex &)
{
	/* Nothing to read */
}


void InsetPagebreak::write(Buffer const &, ostream & os) const
{
	os << "\n" << getCmdName() << '\n';
}


bool InsetPagebreak::metrics(MetricsInfo & mi, Dimension & dim) const
{
	dim.asc = defaultRowHeight();
	dim.des = defaultRowHeight();
	dim.wid = mi.base.textwidth;
	bool const changed = dim_ != dim;
	dim_ = dim;
	return changed;
}


void InsetPagebreak::draw(PainterInfo & pi, int x, int y) const
{
	LyXFont font;
	font.setColor(LColor::pagebreak);
	font.decSize();

	int w = 0;
	int a = 0;
	int d = 0;
	theFontMetrics(font).rectText(insetLabel(), w, a, d);

	int const text_start = int(x + (dim_.wid - w) / 2);
	int const text_end = text_start + w;

	pi.pain.rectText(text_start, y + d, insetLabel(), font,
		LColor::none, LColor::none);

	pi.pain.line(x, y, text_start, y,
		   LColor::pagebreak, Painter::line_onoffdash);
	pi.pain.line(text_end, y, int(x + dim_.wid), y,
		   LColor::pagebreak, Painter::line_onoffdash);
}


int InsetPagebreak::latex(Buffer const &, odocstream & os,
                          OutputParams const &) const
{
	os << from_ascii(getCmdName()) << "{}";
	return 0;
}


int InsetPagebreak::plaintext(Buffer const &, odocstream & os,
                              OutputParams const & runparams) const
{
	os << '\n';
	return PLAINTEXT_NEWLINE;
}


int InsetPagebreak::docbook(Buffer const &, odocstream & os,
                            OutputParams const &) const
{
	os << '\n';
	return 0;
}


} // namespace lyx
