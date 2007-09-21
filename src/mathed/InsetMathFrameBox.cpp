/**
 * \file InsetMathFrameBox.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author André Pönitz
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "InsetMathFrameBox.h"
#include "MathData.h"
#include "MathStream.h"
#include "MathSupport.h"
#include "Color.h"
#include "frontends/Painter.h"


namespace lyx {

InsetMathFrameBox::InsetMathFrameBox()
	: InsetMathNest(3)
{}


Inset * InsetMathFrameBox::clone() const
{
	return new InsetMathFrameBox(*this);
}


void InsetMathFrameBox::metrics(MetricsInfo & mi, Dimension & dim) const
{
	FontSetChanger dummy(mi.base, "textnormal");
	w_ = mathed_char_width(mi.base.font, '[');
	InsetMathNest::metrics(mi);
	dim  = cell(0).dim();
	dim += cell(1).dim();
	dim += cell(2).dim();
	metricsMarkers(dim);
	dim_ = dim;
}


void InsetMathFrameBox::draw(PainterInfo & pi, int x, int y) const
{
	FontSetChanger dummy(pi.base, "textnormal");
	pi.pain.rectangle(x + 1, y - dim_.ascent() + 1,
		dim_.width() - 2, dim_.height() - 2, Color::foreground);
	x += 5;

	drawStrBlack(pi, x, y, from_ascii("["));
	x += w_;
	cell(0).draw(pi, x, y);
	x += cell(0).width();
	drawStrBlack(pi, x, y, from_ascii("]"));
	x += w_ + 4;

	drawStrBlack(pi, x, y, from_ascii("["));
	x += w_;
	cell(1).draw(pi, x, y);
	x += cell(1).width();
	drawStrBlack(pi, x, y, from_ascii("]"));
	x += w_ + 4;

	cell(2).draw(pi, x, y);
	drawMarkers(pi, x, y);
}


void InsetMathFrameBox::write(WriteStream & os) const
{
	os << "\\framebox";
	os << '[' << cell(0) << ']';
	if (cell(1).size())
		os << '[' << cell(1) << ']';
	os << '{' << cell(2) << '}';
}


void InsetMathFrameBox::normalize(NormalStream & os) const
{
	os << "[framebox " << cell(0) << ' ' << cell(1) << ' ' << cell(2) << ']';
}


} // namespace lyx
