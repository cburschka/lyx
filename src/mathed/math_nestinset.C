#ifdef __GNUG__
#pragma implementation
#endif

#include "math_nestinset.h"
#include "math_cursor.h"
#include "math_mathmlstream.h"
#include "formulabase.h"
#include "debug.h"
#include "frontends/Painter.h"

using std::vector;


MathNestInset::MathNestInset(idx_type nargs)
	: MathDimInset(), cells_(nargs), lock_(false)
{}


MathInset::idx_type MathNestInset::nargs() const
{
	return cells_.size();
}


MathXArray & MathNestInset::xcell(idx_type i)
{
	return cells_[i];
}


MathXArray const & MathNestInset::xcell(idx_type i) const
{
	return cells_[i];
}


MathArray & MathNestInset::cell(idx_type i)
{
	return cells_[i].data();
}


MathArray const & MathNestInset::cell(idx_type i) const
{
	return cells_[i].data();
}


void MathNestInset::getPos(idx_type idx, pos_type pos, int & x, int & y) const
{
	MathXArray const & ar = xcell(idx);
	x = ar.xo() + ar.pos2x(pos);
	y = ar.yo();
	// move cursor visually into empty cells ("blue rectangles");
	if (cell(idx).empty())
		x += 2;
}

void MathNestInset::substitute(MathMacro const & m)
{
	for (idx_type i = 0; i < nargs(); ++i)
		cell(i).substitute(m);
}


void MathNestInset::metrics(MathMetricsInfo const & mi) const
{
	MathMetricsInfo m = mi;
	for (idx_type i = 0; i < nargs(); ++i)
		xcell(i).metrics(m);
}


void MathNestInset::metricsMarkers(int frame) const
{
	dim_.d += frame;
	dim_.w += 2 * frame;
}


void MathNestInset::metricsMarkers2(int frame) const
{
	dim_.a += frame;
	dim_.d += frame;
	dim_.w += 2 * frame;
}


bool MathNestInset::idxNext(idx_type & idx, pos_type & pos) const
{
	if (idx + 1 >= nargs())
		return false;
	++idx;
	pos = 0;
	return true;
}


bool MathNestInset::idxRight(idx_type & idx, pos_type & pos) const
{
	return idxNext(idx, pos);
}


bool MathNestInset::idxPrev(idx_type & idx, pos_type & pos) const
{
	if (idx == 0)
		return false;
	--idx;
	pos = cell(idx).size();
	return true;
}


bool MathNestInset::idxLeft(idx_type & idx, pos_type & pos) const
{
	return idxPrev(idx, pos);
}


bool MathNestInset::idxFirst(idx_type & i, pos_type & pos) const
{
	if (nargs() == 0)
		return false;
	i = 0;
	pos = 0;
	return true;
}


bool MathNestInset::idxLast(idx_type & i, pos_type & pos) const
{
	if (nargs() == 0)
		return false;
	i = nargs() - 1;
	pos = cell(i).size();
	return true;
}


bool MathNestInset::idxHome(idx_type & /* idx */, pos_type & pos) const
{
	if (pos == 0)
		return false;
	pos = 0;
	return true;
}


bool MathNestInset::idxEnd(idx_type & idx, pos_type & pos) const
{
	pos_type n = cell(idx).size();
	if (pos == n)
		return false;
	pos = n;
	return true;
}


void MathNestInset::dump() const
{
	WriteStream os(lyxerr);
	os << "---------------------------------------------\n";
	write(os);
	os << "\n";
	for (idx_type i = 0; i < nargs(); ++i)
		os << cell(i) << "\n";
	os << "---------------------------------------------\n";
}


//void MathNestInset::draw(MathPainterInfo & pi, int x, int y) const
void MathNestInset::draw(MathPainterInfo &, int, int) const
{
#if 0
	if (lock_)
		pi.pain.fillRectangle(x, y - ascent(), width(), height(),
					LColor::mathlockbg);
#endif
}


void MathNestInset::drawSelection(MathPainterInfo & pi,
		idx_type idx1, pos_type pos1, idx_type idx2, pos_type pos2) const
{
	if (idx1 == idx2) {
		MathXArray const & c = xcell(idx1);
		int x1 = c.xo() + c.pos2x(pos1);
		int y1 = c.yo() - c.ascent();
		int x2 = c.xo() + c.pos2x(pos2);
		int y2 = c.yo() + c.descent();
		pi.pain.fillRectangle(x1, y1, x2 - x1, y2 - y1, LColor::selection);
	} else {
		for (idx_type i = 0; i < nargs(); ++i) {
			if (idxBetween(i, idx1, idx2)) {
				MathXArray const & c = xcell(i);
				int x1 = c.xo();
				int y1 = c.yo() - c.ascent();
				int x2 = c.xo() + c.width();
				int y2 = c.yo() + c.descent();
				pi.pain.fillRectangle(x1, y1, x2 - x1, y2 - y1, LColor::selection);
			}
		}
	}
}


void MathNestInset::drawMarkers(MathPainterInfo & pi, int x, int y) const
{
	if (!editing())
		return;
	int t = x + width() - 1;
	int d = y + descent();
	pi.pain.line(x, d - 3, x, d, LColor::mathframe);
	pi.pain.line(t, d - 3, t, d, LColor::mathframe);
	pi.pain.line(x, d, x + 3, d, LColor::mathframe);
	pi.pain.line(t - 2, d, t, d, LColor::mathframe);
}


void MathNestInset::drawMarkers2(MathPainterInfo & pi, int x, int y) const
{
	if (!editing())
		return;
	drawMarkers(pi, x, y);
	int t = x + width() - 1;
	int a = y - ascent();
	pi.pain.line(x, a + 3, x, a, LColor::mathframe);
	pi.pain.line(t, a + 3, t, a, LColor::mathframe);
	pi.pain.line(x, a, x + 3, a, LColor::mathframe);
	pi.pain.line(t - 2, a, t, a, LColor::mathframe);
}


void MathNestInset::validate(LaTeXFeatures & features) const
{
	for (idx_type i = 0; i < nargs(); ++i)
		cell(i).validate(features);
}


bool MathNestInset::match(MathInset * p) const
{
	if (nargs() != p->nargs())
		return false;
	for (idx_type i = 0; i < nargs(); ++i)
		if (!cell(i).match(p->cell(i)))
			return false;
	return true;
}


void MathNestInset::replace(ReplaceData & rep)
{
	for (idx_type i = 0; i < nargs(); ++i)
		cell(i).replace(rep);
}


bool MathNestInset::contains(MathArray const & ar)
{
	for (idx_type i = 0; i < nargs(); ++i)
		if (cell(i).contains(ar))
			return true;
	return false;
}


bool MathNestInset::editing() const
{
	return mathcursor && mathcursor->isInside(this);
}


bool MathNestInset::lock() const
{
	return lock_;
}


void MathNestInset::lock(bool l)
{
	lock_ = l;
}


bool MathNestInset::isActive() const
{
	return nargs() > 0;
}


MathArray MathNestInset::glue() const
{
	MathArray ar;
	for (unsigned i = 0; i < nargs(); ++i)
		ar.append(cell(i));
	return ar;
}


void MathNestInset::write(WriteStream & os) const
{
	os << '\\' << name().c_str();
	for (unsigned i = 0; i < nargs(); ++i)
		os << '{' << cell(i) << '}';
	if (lock_ && !os.latex())
		os << "\\lyxlock ";
}


void MathNestInset::normalize(NormalStream & os) const
{
	os << '[' << name().c_str();
	for (unsigned i = 0; i < nargs(); ++i)
		os << ' ' << cell(i);
	os << ']';
}


void MathNestInset::notifyCursorLeaves()
{}
