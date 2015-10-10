/**
 * \file InsetMathXYArrow.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author André Pönitz
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "InsetMathXYArrow.h"
#include "MathStream.h"
#include "MathSupport.h"
#include "frontends/Painter.h"
#include "support/debug.h"

using namespace std;

namespace lyx {


InsetMathXYArrow::InsetMathXYArrow()
	: InsetMathNest(2), up_(false), target_(0)
{}


Inset * InsetMathXYArrow::clone() const
{
	return new InsetMathXYArrow(*this);
}


InsetMathXYMatrix const * InsetMathXYArrow::targetMatrix() const
{
	return target_;
}


MathData const & InsetMathXYArrow::targetCell() const
{
#if 0
	InsetMathXYMatrix const * p = targetMatrix();
	int x = 0;
	int y = 0;
	MathData const & t = cell(0);
	for (MathData::const_iterator it = t.begin(); it != t.end(); ++it) {
		switch ((*it)->getChar()) {
			case 'l': --x; break;
			case 'r': ++x; break;
			case 'u': --y; break;
			case 'd': ++y; break;
		}
	}
	//lyxerr << "target: x: " << x << " y: " << y << endl;
	InsetMath::idx_type n = mi_.idx + p->ncols() * y + x;
	if (n >= p->nargs()) {
		lyxerr << "source: n: " << mi_.idx << "\n"
		       << "target: n: " << n << " out of range" << endl;
		n = 0;
	}
	return p->cell(n);
#else
	static MathData dummy;
	return dummy;
#endif
}


MathData const & InsetMathXYArrow::sourceCell() const
{
#if 0
	return targetMatrix()->cell(mi_.idx);
#else
	static MathData dummy;
	return dummy;
#endif
}


bool InsetMathXYArrow::metrics(MetricsInfo & mi) const
{
	InsetMathNest::metrics(mi);
	mi_   = mi;
	FontSetChanger dummy(mi.base, "textrm");
#if 0
	target_ = mi.inset ? mi.inset->asXYMatrixInset() : 0;

	if (editing()) {
		int w    = mathed_string_width(mi.base.font, from_ascii("target: "));
		width_   = w + max(dim0.width(), dim1.wid);
		ascent_  = dim0.asc;
		descent_ = dim0.des + dim1.height() + 10;
	} else {
		width_   = 0;
		ascent_  = 0;
		descent_ = 0;
		//mathed_string_dim(font_, "X", ascent_, descent_, width_);
	}
#endif
}


void InsetMathXYArrow::draw(PainterInfo & pi, int x, int y) const
{
	metrics(mi_);
	FontSetChanger dummy(pi.base, "textrm");

	if (editing()) {

#if 0

		int lasc;
		int ldes;
		int lwid;
		mathed_string_dim(pi.base.font, "target: ", lasc, ldes, lwid);

		cell(0).draw(pi, x + lwid, y);
		pi.base.text(x + 3, y, "target");
		y += max(dim0.des, ldes) + 5;

		y += max(dim1.asc, lasc) + 5;
		cell(1).draw(pi, x + lwid, y);
		pi.base.text(x + 3, y, "label");

#endif

	} else {

		pi.pain.text(x, y, "X");
		MathData const & s = sourceCell();
		MathData const & t = targetCell();
		pi.pain.line(s.xm(), s.ym(), t.xm(), t.ym(), Color_math);
		cell(1).draw(pi, (s.xm() + t.xm())/2, (s.ym() + t.ym())/2);

	}
}


void InsetMathXYArrow::write(WriteStream & os) const
{
	MathEnsurer ensurer(os);
	os << "\\ar";
	if (!cell(0).empty())
		os << '[' << cell(0) << ']';
	if (!cell(1).empty())
		os << (up_ ? '^' : '_') << '{' << cell(1) << '}';
	os << " ";
}


void InsetMathXYArrow::normalize(NormalStream & os) const
{
	os << "[xyarrow ";
	InsetMathNest::normalize(os);
	os << ']';
}


void InsetMathXYArrow::mathmlize(MathStream &) const
{
	throw MathExportException();
}


void InsetMathXYArrow::htmlize(HtmlStream &) const 
{
	throw MathExportException(); 
}

} // namespace lyx
