/**
 * \file math_frameboxinset.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author André Pönitz
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "math_frameboxinset.h"
#include "math_data.h"
#include "math_mathmlstream.h"
#include "math_support.h"
#include "LColor.h"
#include "frontends/Painter.h"

using std::auto_ptr;


MathFrameboxInset::MathFrameboxInset()
	: MathNestInset(3)
{}


auto_ptr<InsetBase> MathFrameboxInset::clone() const
{
	return auto_ptr<InsetBase>(new MathFrameboxInset(*this));
}


void MathFrameboxInset::metrics(MetricsInfo & mi, Dimension & dim) const
{
	FontSetChanger dummy(mi.base, "textnormal");
	w_ = mathed_char_width(mi.base.font, '[');
	MathNestInset::metrics(mi);
	dim  = cell(0).dim();
	dim += cell(1).dim();
	dim += cell(2).dim();
	metricsMarkers(dim);
	dim_ = dim;
}


void MathFrameboxInset::draw(PainterInfo & pi, int x, int y) const
{
	FontSetChanger dummy(pi.base, "textnormal");
	pi.pain.rectangle(x + 1, y - dim_.ascent() + 1,
		dim_.width() - 2, dim_.height() - 2, LColor::foreground);
	x += 5;

	drawStrBlack(pi, x, y, "[");
	x += w_;
	cell(0).draw(pi, x, y);
	x += cell(0).width();
	drawStrBlack(pi, x, y, "]");
	x += w_ + 4;

	drawStrBlack(pi, x, y, "[");
	x += w_;
	cell(1).draw(pi, x, y);
	x += cell(1).width();
	drawStrBlack(pi, x, y, "]");
	x += w_ + 4;

	cell(2).draw(pi, x, y);
}


void MathFrameboxInset::write(WriteStream & os) const
{
	os << "\\framebox";
	os << '[' << cell(0) << ']';
	if (cell(1).size())
		os << '[' << cell(1) << ']';
	os << '{' << cell(2) << '}';
}


void MathFrameboxInset::normalize(NormalStream & os) const
{
	os << "[framebox " << cell(0) << ' ' << cell(1) << ' ' << cell(2) << ']';
}
