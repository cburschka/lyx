/**
 * \file InsetMathBoxed.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author André Pönitz
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "InsetMathBoxed.h"
#include "MathData.h"
#include "MathStream.h"
#include "LaTeXFeatures.h"

#include "support/std_ostream.h"
#include "frontends/Painter.h"


namespace lyx {

InsetMathBoxed::InsetMathBoxed()
	: InsetMathNest(1)
{}


Inset * InsetMathBoxed::clone() const
{
	return new InsetMathBoxed(*this);
}


void InsetMathBoxed::metrics(MetricsInfo & mi, Dimension & dim) const
{
	cell(0).metrics(mi, dim);
	metricsMarkers2(dim, 3); // 1 pixel space, 1 frame, 1 space
	// Cache the inset dimension. 
	setDimCache(mi, dim);
}


void InsetMathBoxed::draw(PainterInfo & pi, int x, int y) const
{
	Dimension const dim = dimension(*pi.base.bv);
	pi.pain.rectangle(x + 1, y - dim.ascent() + 1,
		dim.width() - 2, dim.height() - 2, Color_foreground);
	cell(0).draw(pi, x + 3, y);
	setPosCache(pi, x, y);
}


void InsetMathBoxed::write(WriteStream & os) const
{
	os << "\\boxed{" << cell(0) << '}';
}


void InsetMathBoxed::normalize(NormalStream & os) const
{
	os << "[boxed " << cell(0) << ']';
}


void InsetMathBoxed::infoize(odocstream & os) const
{
	os << "Boxed: ";
}


void InsetMathBoxed::validate(LaTeXFeatures & features) const
{
	features.require("amsmath");
}


} // namespace lyx
