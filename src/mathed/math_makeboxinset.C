#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include "math_makeboxinset.h"
#include "math_support.h"
#include "math_mathmlstream.h"
#include "math_streamstr.h"
#include "frontends/Painter.h"



MathMakeboxInset::MathMakeboxInset()
	: MathNestInset(3)
{}


MathInset * MathMakeboxInset::clone() const
{
	return new MathMakeboxInset(*this);
}


Dimension MathMakeboxInset::metrics(MetricsInfo & mi) const
{
	FontSetChanger dummy(mi.base, "textnormal");
	w_ = mathed_char_width(mi.base.font, '[');
	MathNestInset::metrics(mi);
	dim_   = cell(0).dim();
	dim_  += cell(1).dim();
	dim_  += cell(2).dim();
	dim_.wid += 4 * w_ + 4;
	metricsMarkers();
	return dim_;
}


void MathMakeboxInset::draw(PainterInfo & pi, int x, int y) const
{
	FontSetChanger dummy(pi.base, "textnormal");
	drawMarkers(pi, x, y);

	drawStrBlack(pi, x, y, "[");
	x += w_;
	cell(0).draw(pi, x, y);
	x += cell(0).width();
	drawStrBlack(pi, x, y, "]");
	x += w_ + 2;

	drawStrBlack(pi, x, y, "[");
	x += w_;
	cell(1).draw(pi, x, y);
	x += cell(1).width();
	drawStrBlack(pi, x, y, "]");
	x += w_ + 2;

	cell(2).draw(pi, x, y);
}


void MathMakeboxInset::write(WriteStream & os) const
{
	os << "\\makebox";
	os << '[' << cell(0) << ']';
	if (cell(1).size())
		os << '[' << cell(1) << ']';
	os << '{' << cell(2) << '}';
}


void MathMakeboxInset::normalize(NormalStream & os) const
{
	os << "[makebox " << cell(0) << ' ' << cell(1) << ' ' << cell(2) << ']';
}
