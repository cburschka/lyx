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

#include "MathData.h"
#include "MathParser.h"
#include "MathStream.h"
#include "MathSupport.h"

#include "Cursor.h"
#include "LaTeXFeatures.h"
#include "MetricsInfo.h"
#include "TextPainter.h"

#include "frontends/FontMetrics.h"
#include "frontends/Painter.h"

#include "support/lassert.h"

using namespace std;


namespace lyx {

/////////////////////////////////////////////////////////////////////
//
// InsetMathFracBase
//
/////////////////////////////////////////////////////////////////////


InsetMathFracBase::InsetMathFracBase(Buffer * buf, idx_type ncells)
	: InsetMathNest(buf, ncells)
{}


bool InsetMathFracBase::idxUpDown(Cursor & cur, bool up) const
{
	// If we only have one cell, target = 0, otherwise
	// target = up ? 0 : 1, since upper cell has idx 0
	idx_type target = nargs() > 1 ? !up : 0;
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


InsetMathFrac::InsetMathFrac(Buffer * buf, Kind kind, idx_type ncells)
	: InsetMathFracBase(buf, ncells), kind_(kind)
{}


Inset * InsetMathFrac::clone() const
{
	return new InsetMathFrac(*this);
}


InsetMathFrac * InsetMathFrac::asFracInset()
{
	return kind_ == ATOP ? nullptr : this;
}


InsetMathFrac const * InsetMathFrac::asFracInset() const
{
	return kind_ == ATOP ? nullptr : this;
}


bool InsetMathFrac::idxForward(Cursor & cur) const
{
	idx_type target = 0;
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
	idx_type target = 0;
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


MathClass InsetMathFrac::mathClass() const
{
	// Generalized fractions are of inner class (see The TeXbook, p. 292)
	// But stuff from the unit/nicefrac packages are not real fractions.
	MathClass mc = MC_ORD;
	switch (kind_) {
	case ATOP:
	case OVER:
	case FRAC:
	case DFRAC:
	case TFRAC:
	case CFRAC:
	case CFRACLEFT:
	case CFRACRIGHT:
		mc = MC_INNER;
		break;
	case NICEFRAC:
	case UNITFRAC:
	case UNIT:
		break;
	}
	return mc;
}



namespace {

// align the top of M in the cell with the top of M in the surrounding font
int dy_for_nicefrac(MetricsBase & mb)
{
	// this is according to nicefrac.sty
	int big_m = theFontMetrics(mb.font).ascent('M');
	Changer dummy = mb.changeScript();
	int small_m = theFontMetrics(mb.font).ascent('M');
	return big_m - small_m;
}


// symbol for nicefrac solidus
latexkeys const * slash_symbol()
{
	return in_word_set(from_ascii("slash"));
}

} // namespace


void InsetMathFrac::metrics(MetricsInfo & mi, Dimension & dim) const
{
	Dimension dim0, dim1, dim2;
	Changer dummy3 = mi.base.changeEnsureMath();

	switch (kind_) {
	case UNIT: {
		// \unitone, \unittwo
		dim.wid = 0;
		int unit_cell = 0;
		// is there an extra cell holding the value being given a dimension?
		// (this is \unittwo)
		if (nargs() == 2) {
			cell(0).metrics(mi, dim1);
			dim.wid += dim1.wid + 4;
			unit_cell = 1;
		}
		Changer dummy = mi.base.font.changeShape(UP_SHAPE);
		cell(unit_cell).metrics(mi, dim0);
		dim.wid += dim0.width() + 1;
		dim.asc = max(dim0.asc, dim1.asc);
		dim.des = max(dim0.des, dim1.des);
	}
		break;

	case UNITFRAC:
	case NICEFRAC: {
		// \unitfrac, \unitfracthree, \nicefrac
		dim.wid = 0;
		dim.asc = 0;
		dim.des = 0;
		int const dy = dy_for_nicefrac(mi.base);
		// is there an extra cell holding the value being given a dimension?
		// (this is \unitfracthree)
		if (kind_ == UNITFRAC && nargs() == 3) {
			cell(2).metrics(mi, dim2);
			dim.wid += dim2.wid + 4;
			dim.asc = dim2.asc;
			dim.des = dim2.des;
		}
		Changer dummy = (kind_ == UNITFRAC) ? mi.base.font.changeShape(UP_SHAPE)
			: noChange();
		Changer dummy2 = mi.base.changeScript();
		if (latexkeys const * slash = slash_symbol()) {
			Dimension dimslash;
			mathedSymbolDim(mi.base, dimslash, slash);
			dim.wid += dimslash.wid - mathed_mu(mi.base.font, 3.0);
			dim.asc = max(dim.asc, dimslash.asc);
			dim.des = max(dim.des, dimslash.des);
		}
		cell(0).metrics(mi, dim0);
		cell(1).metrics(mi, dim1);
		dim.wid += dim0.wid + dim1.wid + 2;
		dim.asc = max(max(dim.asc, dim0.asc + dy), dim1.asc);
		dim.des = max(max(dim.des, dim0.des - dy), dim1.des);
	}
		break;

	case FRAC:
	case CFRAC:
	case CFRACLEFT:
	case CFRACRIGHT:
	case DFRAC:
	case TFRAC:
	case OVER:
	case ATOP: {
		int const dy = axis_height(mi.base);
		Changer dummy =
			// \tfrac is always in text size
			(kind_ == TFRAC) ? mi.base.font.changeStyle(SCRIPT_STYLE) :
			// \cfrac and \dfrac are always in display size
			(kind_ == CFRAC
			 || kind_ == CFRACLEFT
			 || kind_ == CFRACRIGHT
			 || kind_ == DFRAC) ? mi.base.font.changeStyle(DISPLAY_STYLE) :
			// all others
			                      mi.base.changeFrac();
		Changer dummy2 = mi.base.changeEnsureMath();
		cell(0).metrics(mi, dim0);
		cell(1).metrics(mi, dim1);
		dim.wid = max(dim0.wid, dim1.wid) + 2;
		dim.asc = dim0.height() + dy/2 + dy;
		int const t = mi.base.solidLineThickness();
		dim.des = max(0, dim1.height() + dy/2 - dy + t);
	}
	} //switch (kind_)
}


void InsetMathFrac::draw(PainterInfo & pi, int x, int y) const
{
	Changer dummy3 = pi.base.changeEnsureMath();
	Dimension const dim = dimension(*pi.base.bv);
	Dimension const dim0 = cell(0).dimension(*pi.base.bv);
	switch (kind_) {
	case UNIT: {
		// \unitone, \unittwo
		int xx = x;
		int unit_cell = 0;
		// is there an extra cell holding the value being given a dimension?
		// (this is \unittwo)
		if (nargs() == 2) {
			cell(0).draw(pi, x, y);
			xx += dim0.wid + 4;
			unit_cell = 1;
		}
		Changer dummy = pi.base.font.changeShape(UP_SHAPE);
		cell(unit_cell).draw(pi, xx, y);
	}
		break;

	case UNITFRAC:
	case NICEFRAC: {
		// \unitfrac, \unitfracthree, \nicefrac
		int xx = x;
		int const dy = dy_for_nicefrac(pi.base);
		// is there an extra cell holding the value being given a dimension?
		// (this is \unitfracthree)
		if (kind_ == UNITFRAC && nargs() == 3) {
			cell(2).draw(pi, x, y);
			xx += cell(2).dimension(*pi.base.bv).wid + 4;
		}
		Changer dummy = (kind_ == UNITFRAC) ? pi.base.font.changeShape(UP_SHAPE)
			: noChange();
		// nice fraction
		Changer dummy2 = pi.base.changeScript();
		cell(0).draw(pi, xx + 1, y - dy);
		// reference LaTeX code from nicefrac.sty:
		//    \mkern-2mu/\mkern-1mu
		if (latexkeys const * slash = slash_symbol()) {
			int mkern = mathed_mu(pi.base.font, 2.0);
			mathedSymbolDraw(pi, xx + 1 + dim0.wid - mkern, y, slash);
			Dimension dimslash;
			mathedSymbolDim(pi.base, dimslash, slash);
			xx += dimslash.wid - mathed_mu(pi.base.font, 3.0);
		}
		cell(1).draw(pi, xx + 1 + dim0.wid, y);
	}
		break;

	case FRAC:
	case CFRAC:
	case CFRACLEFT:
	case CFRACRIGHT:
	case DFRAC:
	case TFRAC:
	case OVER:
	case ATOP: {
		int const dy = axis_height(pi.base);
		Changer dummy =
			// \tfrac is always in text size
			(kind_ == TFRAC) ? pi.base.font.changeStyle(SCRIPT_STYLE) :
			// \cfrac and \dfrac are always in display size
			(kind_ == CFRAC
			 || kind_ == CFRACLEFT
			 || kind_ == CFRACRIGHT
			 || kind_ == DFRAC) ? pi.base.font.changeStyle(DISPLAY_STYLE) :
			// all others
			                      pi.base.changeFrac();
		Dimension const dim1 = cell(1).dimension(*pi.base.bv);
		int const m = x + dim.wid / 2;
		int const xx =
			// align left
			(kind_ == CFRACLEFT) ? x + 1 :
			// align right
			(kind_ == CFRACRIGHT) ? x + dim.wid - dim0.wid - 2 :
			// center
			                        m - dim0.wid / 2;
		int const t = pi.base.solidLineThickness();
		// take dy/2 for the spacing around the horizontal line. This is
		// arbitrary. In LaTeX it is more complicated to ensure that displayed
		// fractions line up next to each other.
		// For more accurate implementation refer to the TeXbook, Appendix G,
		// rules 15a-e.
		cell(0).draw(pi, xx, y - dim0.des - dy/2 - dy);
		// center
		cell(1).draw(pi, m - dim1.wid / 2, y + dim1.asc + dy/2 - dy + t);
		// horizontal line
		if (kind_ != ATOP)
			pi.pain.line(x, y - dy, x + dim.wid, y - dy,
			             pi.base.font.color(), pi.pain.line_solid, t);
	}
	} //switch (kind_)
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


void InsetMathFrac::write(TeXMathStream & os) const
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
	if (nargs() != 2) {
		// Someone who knows about maple should fix this.
		LASSERT(false, return);
	}
	os << '(' << cell(0) << ")/(" << cell(1) << ')';
}


void InsetMathFrac::mathematica(MathematicaStream & os) const
{
	if (nargs() != 2) {
		// Someone who knows about mathematica should fix this.
		LASSERT(false, return);
	}
	os << '(' << cell(0) << ")/(" << cell(1) << ')';
}


void InsetMathFrac::octave(OctaveStream & os) const
{
	if (nargs() != 2) {
		// Someone who knows about octave should fix this.
		LASSERT(false, return);
	}
	os << '(' << cell(0) << ")/(" << cell(1) << ')';
}


void InsetMathFrac::mathmlize(MathMLStream & ms) const
{
	switch (kind_) {
	case ATOP:
		ms << MTag("mfrac", "linethickness='0'")
		   << MTag("mrow") << cell(0) << ETag("mrow")
		   << MTag("mrow") << cell(1) << ETag("mrow")
		   << ETag("mfrac");
		break;

	// we do not presently distinguish these
	case OVER:
	case FRAC:
	case DFRAC:
	case TFRAC:
	case CFRAC:
	case CFRACLEFT:
	case CFRACRIGHT:
		ms << MTag("mfrac")
		   << MTag("mrow") << cell(0) << ETag("mrow")
		   << MTag("mrow") << cell(1) << ETag("mrow")
		   << ETag("mfrac");
		break;

	case NICEFRAC:
		ms << MTag("mfrac", "bevelled='true'")
		   << MTag("mrow") << cell(0) << ETag("mrow")
		   << MTag("mrow") << cell(1) << ETag("mrow")
		   << ETag("mfrac");
		break;

	case UNITFRAC:
		if (nargs() == 3)
			ms << cell(2);
		ms << MTag("mfrac", "bevelled='true'")
		   << MTag("mrow") << cell(0) << ETag("mrow")
		   << MTag("mrow") << cell(1) << ETag("mrow")
		   << ETag("mfrac");
		break;

	case UNIT:
		// FIXME This is not right, because we still output mi, etc,
		// when we output the cell. So we need to prevent that somehow.
		if (nargs() == 2)
			ms << cell(0)
			   << MTag("mstyle mathvariant='normal'") 
			   << cell(1) 
			   << ETag("mstyle");
		else
			ms << MTag("mstyle mathvariant='normal'")
			   << cell(0)
			   << ETag("mstyle");
	}
}


void InsetMathFrac::htmlize(HtmlStream & os) const
{
	switch (kind_) {
	case ATOP:
		os << MTag("span", "class='frac'")
			 << MTag("span", "class='numer'") << cell(0) << ETag("span")
			 << MTag("span", "class='numer'") << cell(1) << ETag("span")
			 << ETag("span");
		break;

	// we do not presently distinguish these
	case OVER:
	case FRAC:
	case DFRAC:
	case TFRAC:
	case CFRAC:
	case CFRACLEFT:
	case CFRACRIGHT:
		os << MTag("span", "class='frac'")
			 << MTag("span", "class='numer'") << cell(0) << ETag("span")
			 << MTag("span", "class='denom'") << cell(1) << ETag("span")
			 << ETag("span");
		break;

	case NICEFRAC:
		os << cell(0) << '/' << cell(1);
		break;

	case UNITFRAC:
		if (nargs() == 3)
			os << cell(2) << ' ';
		os << cell(0) << '/' << cell(1);
		break;

	case UNIT:
		// FIXME This is not right, because we still output i, etc,
		// when we output the cell. So we need to prevent that somehow.
		if (nargs() == 2)
			os << cell(0)
			   << MTag("span")
			   << cell(1)
			   << ETag("span");
		else
			os << MTag("span")
			   << cell(0)
			   << ETag("span");
	}
}


void InsetMathFrac::validate(LaTeXFeatures & features) const
{
	if (kind_ == NICEFRAC || kind_ == UNITFRAC || kind_ == UNIT)
		features.require("units");
	if (kind_ == CFRAC || kind_ == CFRACLEFT || kind_ == CFRACRIGHT
		  || kind_ == DFRAC || kind_ == TFRAC)
		features.require("amsmath");
	if (features.runparams().math_flavor == OutputParams::MathAsHTML)
		// CSS adapted from eLyXer
		features.addCSSSnippet(
			"span.frac{display: inline-block; vertical-align: middle; text-align:center;}\n"
			"span.numer{display: block;}\n"
			"span.denom{display: block; border-top: thin solid #000040;}");
	InsetMathNest::validate(features);
}


/////////////////////////////////////////////////////////////////////
//
// InsetMathBinom
//
/////////////////////////////////////////////////////////////////////


InsetMathBinom::InsetMathBinom(Buffer * buf, Kind kind)
	: InsetMathFracBase(buf), kind_(kind)
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
	Changer dummy2 = mi.base.changeEnsureMath();
	Dimension dim0, dim1;
	int const dy = axis_height(mi.base);
	Changer dummy =
		(kind_ == DBINOM) ? mi.base.font.changeStyle(DISPLAY_STYLE) :
		(kind_ == TBINOM) ? mi.base.font.changeStyle(SCRIPT_STYLE) :
		                    mi.base.changeFrac();
	cell(0).metrics(mi, dim0);
	cell(1).metrics(mi, dim1);
	dim.asc = dim0.height() + 1 + dy/2 + dy;
	dim.des = max(0, dim1.height() + 1 + dy/2 - dy);
	dim.wid = max(dim0.wid, dim1.wid) + 2 * dw(dim.height()) + 4;
}


void InsetMathBinom::draw(PainterInfo & pi, int x, int y) const
{
	Changer dummy2 = pi.base.changeEnsureMath();
	Dimension const dim = dimension(*pi.base.bv);
	Dimension const & dim0 = cell(0).dimension(*pi.base.bv);
	Dimension const & dim1 = cell(1).dimension(*pi.base.bv);
	int const dy = axis_height(pi.base);
	// define the binom brackets
	docstring const bra = kind_ == BRACE ? from_ascii("{") :
		kind_ == BRACK ? from_ascii("[") : from_ascii("(");
	docstring const ket = kind_ == BRACE ? from_ascii("}") :
		kind_ == BRACK ? from_ascii("]") : from_ascii(")");

	int m = x + dim.width() / 2;
	{
		Changer dummy =
			(kind_ == DBINOM) ? pi.base.font.changeStyle(DISPLAY_STYLE) :
			(kind_ == TBINOM) ? pi.base.font.changeStyle(SCRIPT_STYLE) :
			                    pi.base.changeFrac();
		// take dy both for the vertical alignment and for the spacing between
		// cells
		cell(0).draw(pi, m - dim0.wid / 2, y - dim0.des - dy/2 - dy);
		cell(1).draw(pi, m - dim1.wid / 2, y + dim1.asc + dy/2 - dy);
	}
	// draw the brackets and the marker
	mathed_draw_deco(pi, x, y - dim.ascent(), dw(dim.height()),
		dim.height(), bra);
	mathed_draw_deco(pi, x + dim.width() - dw(dim.height()),
		y - dim.ascent(), dw(dim.height()), dim.height(), ket);
}


bool InsetMathBinom::extraBraces() const
{
	return kind_ == CHOOSE || kind_ == BRACE || kind_ == BRACK;
}


void InsetMathBinom::write(TeXMathStream & os) const
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


void InsetMathBinom::mathmlize(MathMLStream & ms) const
{
	char ldelim = ' ';
	char rdelim = ' ';
	switch (kind_) {
	case BINOM:
	case TBINOM:
	case DBINOM:
	case CHOOSE:
		ldelim = '(';
		rdelim = ')';
		break;
	case BRACE:
		ldelim = '{';
		rdelim = '}';
		break;
	case BRACK:
		ldelim = '[';
		rdelim = ']';
		break;
	}
	ms << "<" << from_ascii(ms.namespacedTag("mo")) << " fence='true' stretchy='true' form='prefix'>"
	   << ldelim
	   << "</" << from_ascii(ms.namespacedTag("mo")) << ">"
	   << "<" << from_ascii(ms.namespacedTag("mfrac")) << " linethickness='0'>"
	   << cell(0) << cell(1)
       << "</" << from_ascii(ms.namespacedTag("mfrac")) << ">"
	   << "<" << from_ascii(ms.namespacedTag("mo")) << " fence='true' stretchy='true' form='postfix'>"
	   << rdelim
	   << "</" << from_ascii(ms.namespacedTag("mo")) << ">";
}


void InsetMathBinom::htmlize(HtmlStream & os) const
{
	char ldelim = ' ';
	char rdelim = ' ';
	switch (kind_) {
	case BINOM:
	case TBINOM:
	case DBINOM:
	case CHOOSE:
		ldelim = '(';
		rdelim = ')';
		break;
	case BRACE:
		ldelim = '{';
		rdelim = '}';
		break;
	case BRACK:
		ldelim = '[';
		rdelim = ']';
		break;
	}
	os << MTag("span", "class='binomdelim'") << ldelim << ETag("span") << '\n'
	   << MTag("span", "class='binom'") << '\n'
	   << MTag("span") << cell(0) << ETag("span") << '\n'
	   << MTag("span") << cell(1) << ETag("span") << '\n'
	   << ETag("span") << '\n'
		 << MTag("span", "class='binomdelim'") << rdelim << ETag("span") << '\n';
}


void InsetMathBinom::validate(LaTeXFeatures & features) const
{
	if (features.runparams().isLaTeX()) {
		if (kind_ == BINOM)
			features.require("binom");
		if (kind_ == DBINOM || kind_ == TBINOM)
			features.require("amsmath");
	} else if (features.runparams().math_flavor == OutputParams::MathAsHTML)
		features.addCSSSnippet(
			"span.binom{display: inline-block; vertical-align: bottom; text-align:center;}\n"
			"span.binom span{display: block;}\n"
			"span.binomdelim{font-size: 2em;}");
	InsetMathNest::validate(features);
}


} // namespace lyx
