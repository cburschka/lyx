/**
 * \file InsetMathFrac.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Alejandro Aguilar Sierra
 * \author André Pönitz
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "InsetMathFrac.h"
#include "MathData.h"
#include "MathStream.h"
#include "TextPainter.h"
#include "LaTeXFeatures.h"
#include "Color.h"
#include "Cursor.h"
#include "frontends/Painter.h"


namespace lyx {

InsetMathFrac::InsetMathFrac(Kind kind, InsetMath::idx_type ncells)
	: InsetMathFracBase(ncells), kind_(kind)
{}


Inset * InsetMathFrac::clone() const
{
	return new InsetMathFrac(*this);
}


InsetMathFrac * InsetMathFrac::asFracInset()
{
	return kind_ == ATOP ? 0 : this;
}


InsetMathFrac const * InsetMathFrac::asFracInset() const
{
	return kind_ == ATOP ? 0 : this;
}


bool InsetMathFrac::idxRight(Cursor & cur) const
{
	InsetMath::idx_type target;
	if (kind_ == UNITFRAC3)
		target = 0;
	else if (kind_ == UNIT) 
		target = 1;
	else
        	return false;
	if (cur.idx() == target)
		return false;
	cur.idx() = target;
	cur.pos() = cell(target).x2pos(cur.x_target());
	return true;
}


bool InsetMathFrac::idxLeft(Cursor & cur) const
{
	InsetMath::idx_type target;
	if (kind_ == UNITFRAC3)
		target = 2;
	else if (kind_ == UNIT) 
		target = 0;
	else
        	return false;
	if (cur.idx() == target)
		return false;
	cur.idx() = target;
	cur.pos() = cell(target).x2pos(cur.x_target());
	return true;
}


bool InsetMathFrac::metrics(MetricsInfo & mi, Dimension & dim) const
{
	if (kind_ == UNIT) {
		cell(0).metrics(mi);
		ShapeChanger dummy2(mi.base.font, Font::UP_SHAPE);
		cell(1).metrics(mi);
		dim.wid = cell(0).width() + cell(1).width() + 5;
		dim.asc = std::max(cell(0).ascent(), cell(1).ascent());
		dim.des = std::max(cell(0).descent(), cell(1).descent());
	} else if (kind_ == UNITFRAC3) {
		cell(2).metrics(mi);
		ShapeChanger dummy2(mi.base.font, Font::UP_SHAPE);
		FracChanger dummy(mi.base);
		cell(0).metrics(mi);
		cell(1).metrics(mi);
		dim.wid = cell(0).width() + cell(1).width() + cell(2).width() + 10;
		dim.asc = std::max(cell(2).ascent(), cell(0).height() + 5);
		dim.des = std::max(cell(2).descent(), cell(1).height() - 5);
	} else {
		FracChanger dummy(mi.base);
		cell(0).metrics(mi);
		cell(1).metrics(mi);
		if (kind_ == NICEFRAC) {
			dim.wid = cell(0).width() + cell(1).width() + 5;
			dim.asc = cell(0).height() + 5;
			dim.des = cell(1).height() - 5;
		} else if (kind_ == UNITFRAC) {
			ShapeChanger dummy2(mi.base.font, Font::UP_SHAPE);
			dim.wid = cell(0).width() + cell(1).width() + 5;
			dim.asc = cell(0).height() + 5;
			dim.des = cell(1).height() - 5;
		} else {
			dim.wid = std::max(cell(0).width(), cell(1).width()) + 2;
			dim.asc = cell(0).height() + 2 + 5;
			dim.des = cell(1).height() + 2 - 5;
		}
	}
	metricsMarkers(dim);
	if (dim_ == dim)
		return false;
	dim_ = dim;
	return true;
}


void InsetMathFrac::draw(PainterInfo & pi, int x, int y) const
{
	setPosCache(pi, x, y);
	int m = x + dim_.wid / 2;
	if (kind_ == UNIT) {
		cell(0).draw(pi, x + 1, y);
		ShapeChanger dummy2(pi.base.font, Font::UP_SHAPE);
		cell(1).draw(pi, x + cell(0).width() + 5, y);
	} else if (kind_ == UNITFRAC3) {
		cell(2).draw(pi, x + 1, y);
		ShapeChanger dummy2(pi.base.font, Font::UP_SHAPE);
		FracChanger dummy(pi.base);
		int xx = x + cell(2).width() + 5;
		cell(0).draw(pi, xx + 2, 
				 y - cell(0).descent() - 5);
		cell(1).draw(pi, xx  + cell(0).width() + 5, 
				 y + cell(1).ascent() / 2);
	} else {
		FracChanger dummy(pi.base);
		if (kind_ == NICEFRAC) {
			cell(0).draw(pi, x + 2,
					y - cell(0).descent() - 5);
			cell(1).draw(pi, x + cell(0).width() + 5,
					y + cell(1).ascent() / 2);
		} else if (kind_ == UNITFRAC) {
			ShapeChanger dummy2(pi.base.font, Font::UP_SHAPE);
			cell(0).draw(pi, x + 2,
					y - cell(0).descent() - 5);
			cell(1).draw(pi, x + cell(0).width() + 5,
					y + cell(1).ascent() / 2);
		} else {
			// Classical fraction
			cell(0).draw(pi, m - cell(0).width() / 2,
					y - cell(0).descent() - 2 - 5);
			cell(1).draw(pi, m - cell(1).width() / 2,
					y + cell(1).ascent()  + 2 - 5);
		}
	}
	if (kind_ == NICEFRAC || kind_ == UNITFRAC || kind_ == UNITFRAC3) {
		// Diag line:
		int xx = x;
		if (kind_ == UNITFRAC3)
			xx += cell(2).width() + 5;
		pi.pain.line(xx + cell(0).width(),
				y + dim_.des - 2,
				xx + cell(0).width() + 5,
				y - dim_.asc + 2, Color::math);
	}
	if (kind_ == FRAC || kind_ == OVER)
		pi.pain.line(x + 1, y - 5,
				x + dim_.wid - 2, y - 5, Color::math);
	drawMarkers(pi, x, y);
}


void InsetMathFrac::metricsT(TextMetricsInfo const & mi, Dimension & dim) const
{
	cell(0).metricsT(mi, dim);
	cell(1).metricsT(mi, dim);
	dim.wid = std::max(cell(0).width(), cell(1).width());
	dim.asc = cell(0).height() + 1;
	dim.des = cell(1).height();
	//dim = dim_;
}


void InsetMathFrac::drawT(TextPainter & pain, int x, int y) const
{
	int m = x + dim_.width() / 2;
	cell(0).drawT(pain, m - cell(0).width() / 2, y - cell(0).descent() - 1);
	cell(1).drawT(pain, m - cell(1).width() / 2, y + cell(1).ascent());
	// ASCII art: ignore niceties
	if (kind_ == FRAC || kind_ == OVER || kind_ == NICEFRAC || kind_ == UNITFRAC)
		pain.horizontalLine(x, y, dim_.width());
}


void InsetMathFrac::write(WriteStream & os) const
{
	switch (kind_) {
	case ATOP:
		os << '{' << cell(0) << "\\atop " << cell(1) << '}';
		break;
	case OVER:
		// \\over is only for compatibility, normalize this to \\frac
		os << "\\frac{" << cell(0) << "}{" << cell(1) << '}';
		break;
	case FRAC:
	case NICEFRAC:
	case UNITFRAC:
		InsetMathNest::write(os);
		break;
	case UNIT:
		os << "\\unit[" << cell(0) << "]{" << cell(1) << '}';
		break;
	case UNITFRAC3:
		os << "\\unitfrac[" << cell(2) << "]{" << cell(0) << "}{" << cell(1) << '}';
		break;
	}
}


docstring InsetMathFrac::name() const
{
	switch (kind_) {
	case FRAC:
		return from_ascii("frac");
	case OVER:
		return from_ascii("over");
	case NICEFRAC:
		return from_ascii("nicefrac");
	case UNITFRAC:
		return from_ascii("unitfrac");
	case UNITFRAC3:
		return from_ascii("unitfracthree");
	case UNIT:
		return from_ascii("unit");
	case ATOP:
		return from_ascii("atop");
	}
	// shut up stupid compiler
	return docstring();
}


bool InsetMathFrac::extraBraces() const
{
	return kind_ == ATOP || kind_ == OVER;
}


void InsetMathFrac::maple(MapleStream & os) const
{
	os << '(' << cell(0) << ")/(" << cell(1) << ')';
}


void InsetMathFrac::mathematica(MathematicaStream & os) const
{
	os << '(' << cell(0) << ")/(" << cell(1) << ')';
}


void InsetMathFrac::octave(OctaveStream & os) const
{
	os << '(' << cell(0) << ")/(" << cell(1) << ')';
}


void InsetMathFrac::mathmlize(MathStream & os) const
{
	os << MTag("mfrac") << cell(0) << cell(1) << ETag("mfrac");
}


void InsetMathFrac::validate(LaTeXFeatures & features) const
{
	if (kind_ == NICEFRAC || kind_ == UNITFRAC 
	 || kind_ == UNIT || kind_ == UNITFRAC3)
		features.require("units");
	InsetMathNest::validate(features);
}



} // namespace lyx
