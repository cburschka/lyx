#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include "math_xyarrowinset.h"
#include "math_xymatrixinset.h"
#include "math_mathmlstream.h"
#include "math_streamstr.h"
#include "math_support.h"
#include "Painter.h"
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
	return mi_.inset ? mi_.inset->asXYMatrixInset() : 0;
}


MathXArray const & MathXYArrowInset::targetCell() const
{
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
	int n = mi_.idx + p->ncols() * y + x;
	if (n < 0 || n >= int(p->nargs())) {
		lyxerr << "source: n: " << mi_.idx << "\n";
		lyxerr << "target: n: " << n << " out of range\n";
		n = 0;
	}
  return p->xcell(n);
}


MathXArray const & MathXYArrowInset::sourceCell() const
{
  return targetMatrix()->xcell(mi_.idx);
}


void MathXYArrowInset::metrics(MathMetricsInfo const & mi) const
{
	mi_ = mi;
	MathNestInset::metrics(mi);

	if (editing()) {
		int w    = mathed_string_width(LM_TC_TEXTRM, mi_, "target: ");
		width_   = w + max(xcell(0).width(), xcell(1).width());
		ascent_  = xcell(0).ascent();
		descent_ = xcell(0).descent() + xcell(1).height() + 10;
	} else {
		width_   = 0;
		ascent_  = 0;
		descent_ = 0;
		//mathed_string_dim(LM_TC_TEXTRM, mi_, "X", ascent_, descent_, width_);
	}
}


void MathXYArrowInset::draw(Painter & pain, int x, int y) const
{
	metrics(mi_);

	if (editing()) {

		int lasc;
		int ldes;
		int lwid;
		mathed_string_dim(LM_TC_TEXTRM, mi_, "target: ", lasc, ldes, lwid);

		xcell(0).draw(pain, x + lwid, y);
		drawStr(pain, LM_TC_TEXTRM, mi_, x + 3, y, "target");
		y += max(xcell(0).descent(), ldes) + 5;

		y += max(xcell(1).ascent(), lasc) + 5;
		xcell(1).draw(pain, x + lwid, y);
		drawStr(pain, LM_TC_TEXTRM, mi_, x + 3, y, "label");

	} else {

		//drawStr(pain, LM_TC_TEXTRM, mi_, x, y, "X");
		MathXArray const & s = sourceCell();
		MathXArray const & t = targetCell();
		pain.line(s.xm(), s.ym(), t.xm(), t.ym(), LColor::math);
		xcell(1).draw(pain, (s.xm() + t.xm())/2, (s.ym() + t.ym())/2);
		
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
