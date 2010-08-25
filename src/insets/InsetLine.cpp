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

#include "Dimension.h"
#include "Font.h"
#include "LaTeXFeatures.h"
#include "MetricsInfo.h"
#include "OutputParams.h"
#include "output_xhtml.h"
#include "Text.h"

#include "frontends/Painter.h"

#include "support/debug.h"
#include "support/docstream.h"



using namespace std;

namespace lyx {

using frontend::Painter;


void InsetLine::read(Lexer &)
{
	/* Nothing to read */
}


void InsetLine::write(ostream & os) const
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
	pi.pain.line(x, y, x + dim.wid, y, Color_foreground,
		Painter::line_solid, Painter::line_medium);
}


int InsetLine::latex(odocstream & os, OutputParams const & runparams) const
{
	os << "\\lyxline{\\"
	   << from_ascii(runparams.local_font->latexSize()) << '}';
	return 0;
}


int InsetLine::plaintext(odocstream & os, OutputParams const &) const
{
	os << "\n-------------------------------------------\n";
	return PLAINTEXT_NEWLINE;
}


int InsetLine::docbook(odocstream & os, OutputParams const &) const
{
	os << '\n';
	return 0;
}


docstring InsetLine::xhtml(XHTMLStream & xs, OutputParams const &) const
{
	xs << html::CompTag("hr");
	xs.cr();
	return docstring();
}


void InsetLine::validate(LaTeXFeatures & features) const
{
	features.require("lyxline");
}


} // namespace lyx
