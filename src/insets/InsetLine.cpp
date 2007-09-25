/**
 * \file InsetLine.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author André Pönitz
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "InsetLine.h"

#include "debug.h"
#include "Color.h"
#include "Text.h"
#include "MetricsInfo.h"
#include "LaTeXFeatures.h"
#include "OutputParams.h"

#include "frontends/Painter.h"


namespace lyx {

using frontend::Painter;

using std::ostream;


void InsetLine::read(Buffer const &, Lexer &)
{
	/* Nothing to read */
}


void InsetLine::write(Buffer const &, ostream & os) const
{
	os << "\n\\lyxline\n";
}


void InsetLine::metrics(MetricsInfo & mi, Dimension & dim) const
{
	dim.asc = 3;
	dim.des = 3;
	dim.wid = mi.base.textwidth;
	// Cache the inset dimension. 
	setDimCache(mi, dim);
}


void InsetLine::draw(PainterInfo & pi, int x, int y) const
{
	Dimension const dim = dimension(*pi.base.bv);
	pi.pain.line(x, y, x + dim.wid, y, Color::topline,
		Painter::line_solid, Painter::line_thick);
}


int InsetLine::latex(Buffer const &, odocstream & os,
		     OutputParams const & runparams) const
{
	os << "\\lyxline{\\"
	   << from_ascii(runparams.local_font->latexSize()) << '}';
	return 0;
}


int InsetLine::plaintext(Buffer const &, odocstream & os,
			 OutputParams const &) const
{
	os << "\n-------------------------------------------\n";
	return PLAINTEXT_NEWLINE;
}


int InsetLine::docbook(Buffer const &, odocstream & os,
		       OutputParams const &) const
{
	os << '\n';
	return 0;
}


void InsetLine::validate(LaTeXFeatures & features) const
{
	features.require("lyxline");
}


} // namespace lyx
