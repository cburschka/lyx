/**
 * \file insetline.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author André Pönitz
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "insetline.h"

#include "debug.h"
#include "LColor.h"
#include "lyxtext.h"
#include "metricsinfo.h"
#include "LaTeXFeatures.h"

#include "frontends/Painter.h"

using std::endl;
using std::ostream;


void InsetLine::read(Buffer const &, LyXLex &)
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
	dim_ = dim;
}


void InsetLine::draw(PainterInfo & pi, int x, int y) const
{
	pi.pain.line(x, y, x + dim_.wid, y, LColor::topline, Painter::line_solid,
			Painter::line_thick);
}


int InsetLine::latex(Buffer const &, ostream & os,
			OutputParams const &) const
{
	os << "\\lyxline{}";

/* was:
	os << "\\lyxline{\\"
		 << pit->getFont(bparams, 0, outerFont(pit, paragraphs)).latexSize()
		 << "}\\vspace{-1\\parskip}";
*/

	return 0;
}


int InsetLine::plaintext(Buffer const &, ostream & os,
		     OutputParams const &) const
{
	os << "-------------------------------------------";
	return 0;
}


int InsetLine::linuxdoc(Buffer const &, std::ostream & os,
			OutputParams const &) const
{
	os << '\n';
	return 0;
}


int InsetLine::docbook(Buffer const &, std::ostream & os,
		       OutputParams const &) const
{
	os << '\n';
	return 0;
}


void InsetLine::validate(LaTeXFeatures & features) const
{
	features.require("lyxline");
}
