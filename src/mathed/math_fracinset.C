/**
 * \file math_fracinset.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Alejandro Aguilar Sierra
 * \author André Pönitz
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "math_fracinset.h"
#include "math_data.h"
#include "math_mathmlstream.h"
#include "textpainter.h"
#include "LColor.h"
#include "frontends/Painter.h"


using std::string;
using std::max;
using std::auto_ptr;


MathFracInset::MathFracInset(bool atop)
	: atop_(atop)
{}


auto_ptr<InsetBase> MathFracInset::clone() const
{
	return auto_ptr<InsetBase>(new MathFracInset(*this));
}


MathFracInset * MathFracInset::asFracInset()
{
	return atop_ ? 0 : this;
}


MathFracInset const * MathFracInset::asFracInset() const
{
	return atop_ ? 0 : this;
}


void MathFracInset::metrics(MetricsInfo & mi, Dimension & dim) const
{
	FracChanger dummy(mi.base);
	cell(0).metrics(mi);
	cell(1).metrics(mi);
	dim_.wid = max(cell(0).width(), cell(1).width()) + 2;
	dim_.asc = cell(0).height() + 2 + 5;
	dim_.des = cell(1).height() + 2 - 5;
	dim = dim_;
}


void MathFracInset::draw(PainterInfo & pi, int x, int y) const
{
	setPosCache(pi, x, y);
	int m = x + dim_.wid / 2;
	FracChanger dummy(pi.base);
	cell(0).draw(pi, m - cell(0).width() / 2, y - cell(0).descent() - 2 - 5);
	cell(1).draw(pi, m - cell(1).width() / 2, y + cell(1).ascent()  + 2 - 5);
	if (!atop_)
		pi.pain.line(x + 1, y - 5, x + dim_.wid - 2, y - 5, LColor::math);
}


void MathFracInset::metricsT(TextMetricsInfo const & mi, Dimension & dim) const
{
	cell(0).metricsT(mi, dim);
	cell(1).metricsT(mi, dim);
	dim.wid = max(cell(0).width(), cell(1).width());
	dim.asc = cell(0).height() + 1;
	dim.des = cell(1).height();
	//dim = dim_;
}


void MathFracInset::drawT(TextPainter & pain, int x, int y) const
{
	int m = x + dim_.width() / 2;
	cell(0).drawT(pain, m - cell(0).width() / 2, y - cell(0).descent() - 1);
	cell(1).drawT(pain, m - cell(1).width() / 2, y + cell(1).ascent());
	if (!atop_)
		pain.horizontalLine(x, y, dim_.width());
}


void MathFracInset::write(WriteStream & os) const
{
	if (atop_)
		os << '{' << cell(0) << "\\atop " << cell(1) << '}';
	else // it's \\frac
		MathNestInset::write(os);
}


string MathFracInset::name() const
{
	return atop_ ? "atop" : "frac";
}


void MathFracInset::maple(MapleStream & os) const
{
	os << '(' << cell(0) << ")/(" << cell(1) << ')';
}


void MathFracInset::mathematica(MathematicaStream & os) const
{
	os << '(' << cell(0) << ")/(" << cell(1) << ')';
}


void MathFracInset::octave(OctaveStream & os) const
{
	os << '(' << cell(0) << ")/(" << cell(1) << ')';
}


void MathFracInset::mathmlize(MathMLStream & os) const
{
	os << MTag("mfrac") << cell(0) << cell(1) << ETag("mfrac");
}
