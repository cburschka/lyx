/**
 * \file InsetMathFBox.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author André Pönitz
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "InsetMathFBox.h"
#include "MathData.h"
#include "MathStream.h"
#include "Color.h"

#include "support/std_ostream.h"
#include "frontends/Painter.h"


namespace lyx {

InsetMathFBox::InsetMathFBox()
	: InsetMathNest(1)
{}


Inset * InsetMathFBox::clone() const
{
	return new InsetMathFBox(*this);
}


InsetMath::mode_type InsetMathFBox::currentMode() const
{
	return TEXT_MODE;
}


bool InsetMathFBox::metrics(MetricsInfo & mi, Dimension & dim) const
{
	FontSetChanger dummy(mi.base, "textnormal");
	cell(0).metrics(mi, dim);
	metricsMarkers(dim, 3); // 1 pixel space, 1 frame, 1 space
	if (dim_ == dim)
		return false;
	dim_ = dim;
	return true;
}


void InsetMathFBox::draw(PainterInfo & pi, int x, int y) const
{
	pi.pain.rectangle(x + 1, y - dim_.ascent() + 1,
		dim_.width() - 2, dim_.height() - 2, Color::foreground);
	FontSetChanger dummy(pi.base, "textnormal");
	cell(0).draw(pi, x + 3, y);
	setPosCache(pi, x, y);
}


void InsetMathFBox::write(WriteStream & os) const
{
	os << "\\fbox{" << cell(0) << '}';
}


void InsetMathFBox::normalize(NormalStream & os) const
{
	os << "[fbox " << cell(0) << ']';
}


void InsetMathFBox::infoize(odocstream & os) const
{
	os << "FBox: ";
}


} // namespace lyx
