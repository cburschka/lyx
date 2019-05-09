/**
 * \file InsetMathRoot.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Alejandro Aguilar Sierra
 * \author André Pönitz
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "InsetMathRoot.h"

#include "MathStream.h"
#include "MathSupport.h"

#include "Cursor.h"
#include "LaTeXFeatures.h"
#include "MetricsInfo.h"

#include "frontends/Painter.h"

#include "support/lassert.h"

using namespace std;

namespace lyx {

using namespace frontend;

InsetMathRoot::InsetMathRoot(Buffer * buf)
	: InsetMathNest(buf, 2)
{}


Inset * InsetMathRoot::clone() const
{
	return new InsetMathRoot(*this);
}


void mathed_root_metrics(MetricsInfo & mi, MathData const & nucleus,
                         MathData const * root, Dimension & dim)
{
	Changer dummy = mi.base.changeEnsureMath();
	Dimension dimr;
	if (root) {
		Changer script = mi.base.font.changeStyle(SCRIPTSCRIPT_STYLE);
		// make sure that the dim is high enough for any character
		root->metrics(mi, dimr, false);
	}

	Dimension dimn;
	nucleus.metrics(mi, dimn);

	// Some room for the decoration
	// The width of left decoration was 9 pixels with a 10em font
	int const w = 9 * mathed_font_em(mi.base.font) / 10;
	/* See rule 11 in Appendix G of Rhe TeXbook for the computation of the spacing
	 * above nucleus.
	 * FIXME more work is needed to implement properly rule 11.
	 * * Ideally, we should use sqrt glyphs from the math fonts. Note
         that then we would get rule thickness from there.
	 * * The positioning of the root MathData is arbitrary. It should
     *   follow the definition of \root...\of... in The Texbook in
     *   Apprendix B page 360.
	 *
	 */
	int const t = mi.base.solidLineThickness();
	int const x_height = mathed_font_x_height(mi.base.font);
	int const phi = (mi.base.font.style() == DISPLAY_STYLE) ? x_height : t;
	// first part is the spacing, second part is the line width
	// itself, and last one is the spacing above.
	int const space_above = (t + phi / 4) + t + t;
	int const a = dimn.ascent();
	int const d = dimn.descent();
	// Not sure what the 1 stands for, it is needed to have some spacing at small sizes.
	dim.asc = max(dimr.ascent() + (d - a) / 2, a + space_above) + 1;
	dim.des = max(dimr.descent() - (d - a) / 2, d);
	dim.wid = max(dimr.width() + 3 * w / 8, w) + dimn.width();
}


void InsetMathRoot::metrics(MetricsInfo & mi, Dimension & dim) const
{
	mathed_root_metrics(mi, cell(0), &cell(1), dim);
}


void mathed_draw_root(PainterInfo & pi, int x, int y, MathData const & nucleus,
                      MathData const * root, Dimension const & dim)
{
	Changer dummy = pi.base.changeEnsureMath();
	// The width of left decoration was 9 pixels with a 10em font
	int const w = 9 * mathed_font_em(pi.base.font) / 10;
	// the height of the hook was 5 with a 10em font
	int const h = 5 * mathed_font_em(pi.base.font) / 10;
	int const a = dim.ascent();
	int const d = dim.descent();
	int const t = pi.base.solidLineThickness();
	Dimension const dimn = nucleus.dimension(*pi.base.bv);
	// the width of the left part of the root
	int const wl = dim.width() - dimn.width();
	// the "exponent"
	if (root) {
		Changer script = pi.base.font.changeStyle(SCRIPTSCRIPT_STYLE);
		Dimension const dimr = root->dimension(*pi.base.bv);
		int const root_offset = wl - 3 * w / 8 - dimr.width();
		root->draw(pi, x + root_offset, y + (d - a)/2);
	}
	// the "base"
	nucleus.draw(pi, x + wl, y);
	int xp[4];
	int yp[4];
	pi.pain.line(x + dim.width(), y - a + 2 * t,
	             x + wl, y - a + 2 * t, pi.base.font.color(),
	             Painter::line_solid, t);
	xp[0] = x + wl;              yp[0] = y - a + 2 * t + 1;
	xp[1] = x + wl - w / 2;      yp[1] = y + d;
	xp[2] = x + wl - w + h / 4;  yp[2] = y + d - h;
	xp[3] = x + wl - w;          yp[3] = y + d - h + h / 4;
	pi.pain.lines(xp, yp, 4, pi.base.font.color(),
	              Painter::fill_none, Painter::line_solid, t);
}


void InsetMathRoot::draw(PainterInfo & pi, int x, int y) const
{
	mathed_draw_root(pi, x, y, cell(0), &cell(1), dimension(*pi.base.bv));
}


void InsetMathRoot::write(WriteStream & os) const
{
	MathEnsurer ensurer(os);
	os << "\\sqrt[" << cell(1) << "]{" << cell(0) << '}';
}


void InsetMathRoot::normalize(NormalStream & os) const
{
	os << "[root " << cell(1) << ' ' << cell(0) << ']';
}


bool InsetMathRoot::idxUpDown(Cursor & cur, bool up) const
{
	Cursor::idx_type const target = up; //up ? 1 : 0;
	if (cur.idx() == target)
		return false;
	cur.idx() = target;
	cur.pos() = up ? cur.lastpos() : 0;
	return true;
}


bool InsetMathRoot::idxForward(Cursor & cur) const
{
	// nucleus is 0 and is on the right
	if (cur.idx() == 0)
		return false;

	cur.idx() = 0;
	cur.pos() = 0;
	return true;
}


bool InsetMathRoot::idxBackward(Cursor & cur) const
{
	// nucleus is 0 and is on the right
	if (cur.idx() == 1)
		return false;

	cur.idx() = 1;
	cur.pos() = cur.lastpos();
	return true;
}


bool InsetMathRoot::idxFirst(Cursor & cur) const
{
	LASSERT(&cur.inset() == this, return false);
	cur.idx() = 1;
	cur.pos() = 0;
	return true;
}


bool InsetMathRoot::idxLast(Cursor & cur) const
{
	LASSERT(&cur.inset() == this, return false);
	cur.idx() = 0;
	cur.pos() = cur.lastpos();
	return true;
}


void InsetMathRoot::maple(MapleStream & os) const
{
	os << '(' << cell(0) << ")^(1/(" << cell(1) <<"))";
}


void InsetMathRoot::mathematica(MathematicaStream & os) const
{
	os << '(' << cell(0) << ")^(1/(" << cell(1) <<"))";
}


void InsetMathRoot::octave(OctaveStream & os) const
{
	os << '(' << cell(0) << ")^(1/(" << cell(1) <<"))";
}


void InsetMathRoot::mathmlize(MathStream & ms) const
{
	ms << MTag("mroot") << cell(0) << cell(1) << ETag("mroot");
}


void InsetMathRoot::htmlize(HtmlStream & os) const
{
	os << MTag("span", "class='root'")
	   << MTag("sup") << cell(1) << ETag("sup")
	   << from_ascii("&radic;")
	   << MTag("span", "class='rootof'")	<< cell(0) << ETag("span")
		 << ETag("span");
}


void InsetMathRoot::validate(LaTeXFeatures & features) const
{
	if (features.runparams().math_flavor == OutputParams::MathAsHTML)
		features.addCSSSnippet(
			"span.rootof{border-top: thin solid black;}\n"
			"span.root sup{font-size: 75%;}");
	InsetMathNest::validate(features);
}

} // namespace lyx
