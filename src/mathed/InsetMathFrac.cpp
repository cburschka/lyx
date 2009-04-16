/**
 * \file InsetMathFracBase.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Alejandro Aguilar Sierra
 * \author André Pönitz
 * \author Uwe Stöhr
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "InsetMathFrac.h"

#include "Cursor.h"
#include "LaTeXFeatures.h"
#include "MathData.h"
#include "MathStream.h"
#include "MathSupport.h"
#include "MetricsInfo.h"
#include "TextPainter.h"

#include "frontends/Painter.h"

using namespace std;

namespace lyx {

/////////////////////////////////////////////////////////////////////
//
// InsetMathFracBase
//
/////////////////////////////////////////////////////////////////////


InsetMathFracBase::InsetMathFracBase(idx_type ncells)
	: InsetMathNest(ncells)
{}


bool InsetMathFracBase::idxUpDown(Cursor & cur, bool up) const
{
	InsetMath::idx_type target = !up; // up ? 0 : 1, since upper cell has idx 0
	if (cur.idx() == target)
		return false;
	cur.idx() = target;
	cur.pos() = cell(target).x2pos(&cur.bv(), cur.x_target());
	return true;
}



/////////////////////////////////////////////////////////////////////
//
// InsetMathFrac
//
/////////////////////////////////////////////////////////////////////


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


bool InsetMathFrac::idxForward(Cursor & cur) const
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
	cur.pos() = cell(target).x2pos(&cur.bv(), cur.x_target());
	return true;
}


bool InsetMathFrac::idxBackward(Cursor & cur) const
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
	cur.pos() = cell(target).x2pos(&cur.bv(), cur.x_target());
	return true;
}


void InsetMathFrac::metrics(MetricsInfo & mi, Dimension & dim) const
{
	Dimension dim0, dim1, dim2;

	if (kind_ == UNIT || (kind_ == UNITFRAC && nargs() == 3)) {
		if (nargs() == 1) {
			ShapeChanger dummy2(mi.base.font, UP_SHAPE);
			cell(0).metrics(mi, dim0);
			dim.wid = dim0.width()+ 3;
			dim.asc = dim0.asc;
			dim.des = dim0.des;
		} else if (nargs() == 2) {
			cell(0).metrics(mi, dim0);
			ShapeChanger dummy2(mi.base.font, UP_SHAPE);
			cell(1).metrics(mi, dim1);
			dim.wid = dim0.width() + dim1.wid + 5;
			dim.asc = max(dim0.asc, dim1.asc);
			dim.des = max(dim0.des, dim1.des);
		} else {
			cell(2).metrics(mi, dim2);
			ShapeChanger dummy2(mi.base.font, UP_SHAPE);
			FracChanger dummy(mi.base);
			cell(0).metrics(mi, dim0);
			cell(1).metrics(mi, dim1);
			dim.wid = dim0.width() + dim1.wid + dim2.wid + 10;
			dim.asc = max(dim2.asc, dim0.height() + 5);
			dim.des = max(dim2.des, dim1.height() - 5);
		}
	} else {
		// general cell metrics used for \frac
		FracChanger dummy(mi.base);
		cell(0).metrics(mi, dim0);
		cell(1).metrics(mi, dim1);
		if (nargs() == 3)
			cell(2).metrics(mi, dim2);
		// metrics for special fraction types
		if (kind_ == NICEFRAC) {
			dim.wid = dim0.width() + dim1.wid + 5;
			dim.asc = dim0.height() + 5;
			dim.des = dim1.height() - 5;
		} else if (kind_ == UNITFRAC) {
			ShapeChanger dummy2(mi.base.font, UP_SHAPE);
			dim.wid = dim0.width() + dim1.wid + 5;
			dim.asc = dim0.height() + 5;
			dim.des = dim1.height() - 5;
		} else {
			if (kind_ == CFRAC || kind_ == CFRACLEFT
			|| kind_ == CFRACRIGHT || kind_ == DFRAC) {
				// \cfrac and \dfrac are always in display size
				StyleChanger dummy2(mi.base, LM_ST_DISPLAY);
				cell(0).metrics(mi, dim0);
				cell(1).metrics(mi, dim1);
			} else if (kind_ == TFRAC) {
				// tfrac is in always in text size
				StyleChanger dummy2(mi.base, LM_ST_SCRIPT);
				cell(0).metrics(mi, dim0);
				cell(1).metrics(mi, dim1);
			}
			dim.wid = max(dim0.wid, dim1.wid) + 2;
			dim.asc = dim0.height() + 2 + 5;
			dim.des = dim1.height() + 2 - 5;
		}
	}
	metricsMarkers(dim);
}


void InsetMathFrac::draw(PainterInfo & pi, int x, int y) const
{
	setPosCache(pi, x, y);
	Dimension const dim = dimension(*pi.base.bv);
	Dimension const dim0 = cell(0).dimension(*pi.base.bv);
	if (kind_ == UNIT || (kind_ == UNITFRAC && nargs() == 3)) {
		if (nargs() == 1) {
			ShapeChanger dummy2(pi.base.font, UP_SHAPE);
			cell(0).draw(pi, x + 1, y);
		} else if (nargs() == 2) {
			cell(0).draw(pi, x + 1, y);
			ShapeChanger dummy2(pi.base.font, UP_SHAPE);
			cell(1).draw(pi, x + dim0.width() + 5, y);
		} else {
			cell(2).draw(pi, x + 1, y);
			ShapeChanger dummy2(pi.base.font, UP_SHAPE);
			FracChanger dummy(pi.base);
			Dimension const dim1 = cell(1).dimension(*pi.base.bv);
			Dimension const dim2 = cell(2).dimension(*pi.base.bv);
			int xx = x + dim2.wid + 5;
			cell(0).draw(pi, xx + 2, 
					 y - dim0.des - 5);
			cell(1).draw(pi, xx  + dim0.width() + 5, 
					 y + dim1.asc / 2);
		}
	} else {
		FracChanger dummy(pi.base);
		Dimension const dim1 = cell(1).dimension(*pi.base.bv);
		int m = x + dim.wid / 2;
		if (kind_ == NICEFRAC) {
			cell(0).draw(pi, x + 2,
					y - dim0.des - 5);
			cell(1).draw(pi, x + dim0.width() + 5,
					y + dim1.asc / 2);
		} else if (kind_ == UNITFRAC) {
			ShapeChanger dummy2(pi.base.font, UP_SHAPE);
			cell(0).draw(pi, x + 2,	y - dim0.des - 5);
			cell(1).draw(pi, x + dim0.width() + 5, y + dim1.asc / 2);
		} else if (kind_ == FRAC) {
			cell(0).draw(pi, m - dim0.wid / 2, y - dim0.des - 2 - 5);
			cell(1).draw(pi, m - dim1.wid / 2, y + dim1.asc + 2 - 5);
		} else if (kind_ == TFRAC) {
			// tfrac is in always in text size
			StyleChanger dummy2(pi.base, LM_ST_SCRIPT);
			cell(0).draw(pi, m - dim0.wid / 2, y - dim0.des - 2 - 5);
			cell(1).draw(pi, m - dim1.wid / 2, y + dim1.asc + 2 - 5);
		} else {
			// \cfrac and \dfrac are always in display size
			StyleChanger dummy2(pi.base, LM_ST_DISPLAY);
			if (kind_ == CFRAC || kind_ == DFRAC)
				cell(0).draw(pi, m - dim0.wid / 2, y - dim0.des - 2 - 5);
			else if (kind_ == CFRACLEFT)
				cell(0).draw(pi, x + 2, y - dim0.des - 2 - 5);
			else if (kind_ == CFRACRIGHT)
				cell(0).draw(pi, x + dim.wid - dim0.wid - 2,
					y - dim0.des - 2 - 5);
			cell(1).draw(pi, m - dim1.wid / 2, y + dim1.asc + 2 - 5);
		}
	}
	if (kind_ == NICEFRAC || kind_ == UNITFRAC) {
		// Diag line:
		int xx = x;
		if (nargs() == 3)
			xx += cell(2).dimension(*pi.base.bv).wid + 5;
		pi.pain.line(xx + dim0.wid,
				y + dim.des - 2,
				xx + dim0.wid + 5,
				y - dim.asc + 2, Color_math);
	}
	if (kind_ == FRAC || kind_ == CFRAC || kind_ == CFRACLEFT
		|| kind_ == CFRACRIGHT || kind_ == DFRAC
		|| kind_ == TFRAC || kind_ == OVER)
		pi.pain.line(x + 1, y - 5,
				x + dim.wid - 2, y - 5, Color_math);
	drawMarkers(pi, x, y);
}


void InsetMathFrac::metricsT(TextMetricsInfo const & mi, Dimension & dim) const
{
	Dimension dim0, dim1;
	cell(0).metricsT(mi, dim0);
	cell(1).metricsT(mi, dim1);
	dim.wid = max(dim0.width(), dim1.wid);
	dim.asc = dim0.height() + 1;
	dim.des = dim1.height();
}


void InsetMathFrac::drawT(TextPainter & /*pain*/, int /*x*/, int /*y*/) const
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
	MathEnsurer ensurer(os);
	switch (kind_) {
	case ATOP:
		// \\atop is only for compatibility, \\binom is the
		// LaTeX2e successor
		os << '{' << cell(0) << "\\atop " << cell(1) << '}';
		break;
	case OVER:
		// \\over is only for compatibility, normalize this to \\frac
		os << "\\frac{" << cell(0) << "}{" << cell(1) << '}';
		break;
	case FRAC:
	case DFRAC:
	case TFRAC:
	case NICEFRAC:
	case CFRAC:
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
	case CFRACLEFT:
		os << "\\cfrac[l]{" << cell(0) << "}{" << cell(1) << '}';
		break;
	case CFRACRIGHT:
		os << "\\cfrac[r]{" << cell(0) << "}{" << cell(1) << '}';
		break;
	}
}


docstring InsetMathFrac::name() const
{
	switch (kind_) {
	case FRAC:
		return from_ascii("frac");
	case CFRAC:
	case CFRACLEFT:
	case CFRACRIGHT:
		return from_ascii("cfrac");
	case DFRAC:
		return from_ascii("dfrac");
	case TFRAC:
		return from_ascii("tfrac");
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
	switch (kind_) {
	case DFRAC:
		os << MTag("mdfrac") << cell(0) << cell(1) << ETag("mdfrac");
		break;
	case TFRAC:
		os << MTag("mtfrac") << cell(0) << cell(1) << ETag("mtfrac");
		break;
	case FRAC:
	default:
		os << MTag("mfrac") << cell(0) << cell(1) << ETag("mfrac");
		break;
	}
}


void InsetMathFrac::validate(LaTeXFeatures & features) const
{
	if (kind_ == NICEFRAC || kind_ == UNITFRAC || kind_ == UNIT)
		features.require("units");
	if (kind_ == CFRAC || kind_ == CFRACLEFT || kind_ == CFRACRIGHT
		|| kind_ == DFRAC || kind_ == TFRAC)
		features.require("amsmath");
	InsetMathNest::validate(features);
}


/////////////////////////////////////////////////////////////////////
//
// InsetMathBinom
//
/////////////////////////////////////////////////////////////////////


InsetMathBinom::InsetMathBinom(Kind kind)
	: kind_(kind)
{}


Inset * InsetMathBinom::clone() const
{
	return new InsetMathBinom(*this);
}


int InsetMathBinom::dw(int height) const
{
	int w = height / 5;
	if (w > 15)
		w = 15;
	if (w < 6)
		w = 6;
	return w;
}


void InsetMathBinom::metrics(MetricsInfo & mi, Dimension & dim) const
{
	Dimension dim0, dim1;

	// FIXME: for an unknown reason the cells must be set directly
	// after the StyleChanger and cannot be set after the if case
	if (kind_ == DBINOM) {
		StyleChanger dummy(mi.base, LM_ST_DISPLAY);
		cell(0).metrics(mi, dim0);
		cell(1).metrics(mi, dim1);
	} else if (kind_ == TBINOM) {
		StyleChanger dummy(mi.base, LM_ST_SCRIPT);
		cell(0).metrics(mi, dim0);
		cell(1).metrics(mi, dim1);
	} else {
		FracChanger dummy(mi.base);
		cell(0).metrics(mi, dim0);
		cell(1).metrics(mi, dim1);
	}
	dim.asc = dim0.height() + 4 + 5;
	dim.des = dim1.height() + 4 - 5;
	dim.wid = max(dim0.wid, dim1.wid) + 2 * dw(dim.height()) + 4;
	metricsMarkers2(dim);
}


void InsetMathBinom::draw(PainterInfo & pi, int x, int y) const
{
	Dimension const dim = dimension(*pi.base.bv);
	Dimension const & dim0 = cell(0).dimension(*pi.base.bv);
	Dimension const & dim1 = cell(1).dimension(*pi.base.bv);
	// define the binom brackets
	docstring const bra = kind_ == BRACE ? from_ascii("{") :
		kind_ == BRACK ? from_ascii("[") : from_ascii("(");
	docstring const ket = kind_ == BRACE ? from_ascii("}") :
		kind_ == BRACK ? from_ascii("]") : from_ascii(")");

	int m = x + dim.width() / 2;
	// FIXME: for an unknown reason the cells must be drawn directly
	// after the StyleChanger and cannot be drawn after the if case
	if (kind_ == DBINOM) {
		StyleChanger dummy(pi.base, LM_ST_DISPLAY);
		cell(0).draw(pi, m - dim0.wid / 2, y - dim0.des - 3 - 5);
		cell(1).draw(pi, m - dim1.wid / 2, y + dim1.asc + 3 - 5);
	} else if (kind_ == TBINOM) {
		StyleChanger dummy(pi.base, LM_ST_SCRIPT);
		cell(0).draw(pi, m - dim0.wid / 2, y - dim0.des - 3 - 5);
		cell(1).draw(pi, m - dim1.wid / 2, y + dim1.asc + 3 - 5);
	} else {
		FracChanger dummy2(pi.base);
		cell(0).draw(pi, m - dim0.wid / 2, y - dim0.des - 3 - 5);
		cell(1).draw(pi, m - dim1.wid / 2, y + dim1.asc + 3 - 5);
	}
	// draw the brackets and the marker
	mathed_draw_deco(pi, x, y - dim.ascent(), dw(dim.height()),
		dim.height(), bra);
	mathed_draw_deco(pi, x + dim.width() - dw(dim.height()),
		y - dim.ascent(), dw(dim.height()), dim.height(), ket);
	drawMarkers2(pi, x, y);
}


bool InsetMathBinom::extraBraces() const
{
	return kind_ == CHOOSE || kind_ == BRACE || kind_ == BRACK;
}


void InsetMathBinom::write(WriteStream & os) const
{
	MathEnsurer ensurer(os);
	switch (kind_) {
	case BINOM:
		os << "\\binom{" << cell(0) << "}{" << cell(1) << '}';
		break;
	case DBINOM:
		os << "\\dbinom{" << cell(0) << "}{" << cell(1) << '}';
		break;
	case TBINOM:
		os << "\\tbinom{" << cell(0) << "}{" << cell(1) << '}';
		break;
	case CHOOSE:
		os << '{' << cell(0) << " \\choose " << cell(1) << '}';
		break;
	case BRACE:
		os << '{' << cell(0) << " \\brace " << cell(1) << '}';
		break;
	case BRACK:
		os << '{' << cell(0) << " \\brack " << cell(1) << '}';
		break;
	}
}


void InsetMathBinom::normalize(NormalStream & os) const
{
	os << "[binom " << cell(0) << ' ' << cell(1) << ']';
}


void InsetMathBinom::mathmlize(MathStream & os) const
{
	switch (kind_) {
	case BINOM:
		os << MTag("mbinom") << cell(0) << cell(1) << ETag("mbinom");
		break;
	case TBINOM:
		os << MTag("mtbinom") << cell(0) << cell(1) << ETag("mtbinom");
		break;
	case DBINOM:
	default:
		os << MTag("mdbinom") << cell(0) << cell(1) << ETag("mdbinom");
		break;
	}
}


void InsetMathBinom::validate(LaTeXFeatures & features) const
{
	if (kind_ == BINOM)
		features.require("binom");
	if (kind_ == DBINOM || kind_ == TBINOM)
		features.require("amsmath");
	InsetMathNest::validate(features);
}


} // namespace lyx
