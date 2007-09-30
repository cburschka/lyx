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
	InsetMath::idx_type target = 0;
	if (kind_ == UNIT || (kind_ == UNITFRAC && nargs() == 3)) {
		if (nargs() == 3)
			target = 0;
		else if (nargs() == 2)
			target = 1;
	} else
		return false;
	if (cur.idx() == target)
		return false;
	cur.idx() = target;
	cur.pos() = cell(target).x2pos(cur.x_target());
	return true;
}


bool InsetMathFrac::idxLeft(Cursor & cur) const
{
	InsetMath::idx_type target = 0;
	if (kind_ == UNIT || (kind_ == UNITFRAC && nargs() == 3)) {
		if (nargs() == 3)
			target = 2;
		else if (nargs() == 2)
			target = 0;
	} else
		return false;
	if (cur.idx() == target)
		return false;
	cur.idx() = target;
	cur.pos() = cell(target).x2pos(cur.x_target());
	return true;
}


void InsetMathFrac::metrics(MetricsInfo & mi, Dimension & dim) const
{
	Dimension dim0, dim1, dim2;

	if (kind_ == UNIT || (kind_ == UNITFRAC && nargs() == 3)) {
		if (nargs() == 1) {
			ShapeChanger dummy2(mi.base.font, Font::UP_SHAPE);
			cell(0).metrics(mi, dim0);
			dim.wid = dim0.width()+ 3;
			dim.asc = dim0.asc;
			dim.des = dim0.des;
		} else if (nargs() == 2) {
			cell(0).metrics(mi, dim0);
			ShapeChanger dummy2(mi.base.font, Font::UP_SHAPE);
			cell(1).metrics(mi, dim1);
			dim.wid = dim0.width() + dim1.wid + 5;
			dim.asc = std::max(dim0.asc, dim1.asc);
			dim.des = std::max(dim0.des, dim1.des);
		} else {
			cell(2).metrics(mi, dim2);
			ShapeChanger dummy2(mi.base.font, Font::UP_SHAPE);
			FracChanger dummy(mi.base);
			cell(0).metrics(mi, dim0);
			cell(1).metrics(mi, dim1);
			dim.wid = dim0.width() + dim1.wid + dim2.wid + 10;
			dim.asc = std::max(dim2.asc, dim0.height() + 5);
			dim.des = std::max(dim2.des, dim1.height() - 5);
		}
	} else {
		FracChanger dummy(mi.base);
		cell(0).metrics(mi, dim0);
		cell(1).metrics(mi, dim1);
		if (kind_ == NICEFRAC) {
			dim.wid = dim0.width() + dim1.wid + 5;
			dim.asc = dim0.height() + 5;
			dim.des = dim1.height() - 5;
		} else if (kind_ == UNITFRAC) {
			ShapeChanger dummy2(mi.base.font, Font::UP_SHAPE);
			dim.wid = dim0.width() + dim1.wid + 5;
			dim.asc = dim0.height() + 5;
			dim.des = dim1.height() - 5;
		} else {
			dim.wid = std::max(dim0.width(), dim1.wid) + 2;
			dim.asc = dim0.height() + 2 + 5;
			dim.des = dim1.height() + 2 - 5;
		}
	}
	metricsMarkers(dim);
	// Cache the inset dimension. 
	setDimCache(mi, dim);
}


void InsetMathFrac::draw(PainterInfo & pi, int x, int y) const
{
	setPosCache(pi, x, y);
	Dimension const dim = dimension(*pi.base.bv);
	Dimension const dim0 = cell(0).dimension(*pi.base.bv);
	Dimension const dim1 = cell(1).dimension(*pi.base.bv);
	Dimension const dim2 = cell(0).dimension(*pi.base.bv);
	int m = x + dim.wid / 2;
	if (kind_ == UNIT || (kind_ == UNITFRAC && nargs() == 3)) {
		if (nargs() == 1) {
			ShapeChanger dummy2(pi.base.font, Font::UP_SHAPE);
			cell(0).draw(pi, x + 1, y);
		} else if (nargs() == 2) {
			cell(0).draw(pi, x + 1, y);
			ShapeChanger dummy2(pi.base.font, Font::UP_SHAPE);
			cell(1).draw(pi, x + dim0.width() + 5, y);
		} else {
			cell(2).draw(pi, x + 1, y);
			ShapeChanger dummy2(pi.base.font, Font::UP_SHAPE);
			FracChanger dummy(pi.base);
			int xx = x + dim2.wid + 5;
			cell(0).draw(pi, xx + 2, 
					 y - dim0.des - 5);
			cell(1).draw(pi, xx  + dim0.width() + 5, 
					 y + dim1.asc / 2);
		}
	} else {
		FracChanger dummy(pi.base);
		if (kind_ == NICEFRAC) {
			cell(0).draw(pi, x + 2,
					y - dim0.des - 5);
			cell(1).draw(pi, x + dim0.width() + 5,
					y + dim1.asc / 2);
		} else if (kind_ == UNITFRAC) {
			ShapeChanger dummy2(pi.base.font, Font::UP_SHAPE);
			cell(0).draw(pi, x + 2,
					y - dim0.des - 5);
			cell(1).draw(pi, x + dim0.width() + 5,
					y + dim1.asc / 2);
		} else {
			// Classical fraction
			cell(0).draw(pi, m - dim0.width() / 2,
					y - dim0.des - 2 - 5);
			cell(1).draw(pi, m - dim1.wid / 2,
					y + dim1.asc  + 2 - 5);
		}
	}
	if (kind_ == NICEFRAC || kind_ == UNITFRAC) {
		// Diag line:
		int xx = x;
		if (nargs() == 3)
			xx += dim2.wid + 5;
		pi.pain.line(xx + dim0.wid,
				y + dim.des - 2,
				xx + dim0.wid + 5,
				y - dim.asc + 2, Color::math);
	}
	if (kind_ == FRAC || kind_ == OVER)
		pi.pain.line(x + 1, y - 5,
				x + dim.wid - 2, y - 5, Color::math);
	drawMarkers(pi, x, y);
}


void InsetMathFrac::metricsT(TextMetricsInfo const & mi, Dimension & dim) const
{
	Dimension dim0, dim1;
	cell(0).metricsT(mi, dim0);
	cell(1).metricsT(mi, dim1);
	dim.wid = std::max(dim0.width(), dim1.wid);
	dim.asc = dim0.height() + 1;
	dim.des = dim1.height();
}


void InsetMathFrac::drawT(TextPainter & pain, int x, int y) const
{
	// FIXME: BROKEN!
	/*
	Dimension dim;
	int m = x + dim.width() / 2;
	cell(0).drawT(pain, m - dim0.width() / 2, y - dim0.des - 1);
	cell(1).drawT(pain, m - dim1.wid / 2, y + dim1.asc);
	// ASCII art: ignore niceties
	if (kind_ == FRAC || kind_ == OVER || kind_ == NICEFRAC || kind_ == UNITFRAC)
		pain.horizontalLine(x, y, dim.width());
	*/
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
		if (nargs() == 2)
			InsetMathNest::write(os);
		else
			os << "\\unitfrac[" << cell(2) << "]{" << cell(0) << "}{" << cell(1) << '}';
		break;
	case UNIT:
		if (nargs() == 2)
			os << "\\unit[" << cell(0) << "]{" << cell(1) << '}';
		else
			os << "\\unit{" << cell(0) << '}';
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
	if (kind_ == NICEFRAC || kind_ == UNITFRAC || kind_ == UNIT)
		features.require("units");
	InsetMathNest::validate(features);
}



} // namespace lyx
