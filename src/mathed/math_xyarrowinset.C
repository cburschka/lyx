#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include "math_xyarrowinset.h"
#include "math_xymatrixinset.h"
#include "math_mathmlstream.h"
#include "math_streamstr.h"
#include "math_support.h"
#include "frontends/Painter.h"
#include "debug.h"


using std::max;


MathXYArrowInset::MathXYArrowInset()
	: MathNestInset(2)
{}


MathInset * MathXYArrowInset::clone() const
{
	return new MathXYArrowInset(*this);
}


MathXYMatrixInset const * MathXYArrowInset::targetMatrix() const
{
	return target_;
}


MathArray const & MathXYArrowInset::targetCell() const
{
#if 0	
	MathXYMatrixInset const * p = targetMatrix();
	int x = 0;
	int y = 0;
	MathArray const & t = cell(0);
	for (MathArray::const_iterator it = t.begin(); it != t.end(); ++it) {
		switch ((*it)->getChar()) {
			case 'l': --x; break;
			case 'r': ++x; break;
			case 'u': --y; break;
			case 'd': ++y; break;
		}
	}
	//lyxerr << "target: x: " << x << " y: " << y << "\n";
	MathInset::idx_type n = mi_.idx + p->ncols() * y + x;
	if (n >= p->nargs()) {
		lyxerr << "source: n: " << mi_.idx << "\n";
		lyxerr << "target: n: " << n << " out of range\n";
		n = 0;
	}
  return p->cell(n);
#else
	static MathArray dummy;
	return dummy;
#endif
}


MathArray const & MathXYArrowInset::sourceCell() const
{
#if 0
  return targetMatrix()->cell(mi_.idx);
#else
	static MathArray dummy;
	return dummy;
#endif
}


void MathXYArrowInset::metrics(MathMetricsInfo & mi) const
{
	MathNestInset::metrics(mi);
	mi_   = mi;
	MathFontSetChanger dummy(mi.base, "textrm");
#if 0
	target_ = mi.inset ? mi.inset->asXYMatrixInset() : 0;

	if (editing()) {
		int w    = mathed_string_width(mi.base.font, "target: ");
		width_   = w + max(cell(0).width(), cell(1).width());
		ascent_  = cell(0).ascent();
		descent_ = cell(0).descent() + cell(1).height() + 10;
	} else {
		width_   = 0;
		ascent_  = 0;
		descent_ = 0;
		//mathed_string_dim(font_, "X", ascent_, descent_, width_);
	}
#endif
}


void MathXYArrowInset::draw(MathPainterInfo & pi, int x, int y) const
{
	metrics(mi_);
	MathFontSetChanger dummy(pi.base, "textrm");

	if (editing()) {

#if 0

		int lasc;
		int ldes;
		int lwid;
		mathed_string_dim(pi.base.font, "target: ", lasc, ldes, lwid);

		cell(0).draw(pi, x + lwid, y);
		drawStr(pi, pi.base.font, x + 3, y, "target");
		y += max(cell(0).descent(), ldes) + 5;

		y += max(cell(1).ascent(), lasc) + 5;
		cell(1).draw(pi, x + lwid, y);
		drawStr(pi, pi.base.font, x + 3, y, "label");

#endif

	} else {

		drawStr(pi, font_, x, y, "X");
		MathArray const & s = sourceCell();
		MathArray const & t = targetCell();
		pi.pain.line(s.xm(), s.ym(), t.xm(), t.ym(), LColor::math);
		cell(1).draw(pi, (s.xm() + t.xm())/2, (s.ym() + t.ym())/2);

	}
}


void MathXYArrowInset::write(WriteStream & os) const
{
	os << "\\ar";
	if (cell(0).size())
		os << "[" << cell(0) << "]";
	if (cell(1).size())
		os << (up_ ? "^" : "_") << "{" << cell(1) << "}";
	os << " ";
}


void MathXYArrowInset::normalize(NormalStream & os) const
{
	os << "[xyarrow ";
	MathNestInset::normalize(os);
	os << "]";
}
