#include <config.h>

#include "math_frameboxinset.h"
#include "math_support.h"
#include "math_mathmlstream.h"
#include "math_streamstr.h"
#include "frontends/Painter.h"



MathFrameboxInset::MathFrameboxInset()
	: MathNestInset(3)
{}


MathInset * MathFrameboxInset::clone() const
{
	return new MathFrameboxInset(*this);
}


void MathFrameboxInset::metrics(MathMetricsInfo & mi) const
{
	MathFontSetChanger dummy(mi.base, "textnormal");
	w_ = mathed_char_width(mi.base.font, '[');
	MathNestInset::metrics(mi);
	dim_    = cell(0).dim();
	dim_   += cell(1).dim();
	dim_   += cell(2).dim();
	dim_.w += 4 * w_ + 4;
	metricsMarkers2(5); // 5 pixels margin
}


void MathFrameboxInset::draw(MathPainterInfo & pi, int x, int y) const
{
	MathFontSetChanger dummy(pi.base, "textnormal");
	pi.pain.rectangle(x + 1, y - ascent() + 1, width() - 2, height() - 2,
			LColor::black);
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
