#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include "math_data.h"
#include "math_inset.h"
#include "math_deliminset.h"
#include "math_charinset.h"
#include "math_scriptinset.h"
#include "math_stringinset.h"
#include "math_matrixinset.h"
#include "math_mathmlstream.h"
#include "math_support.h"
#include "math_replace.h"
#include "debug.h"
#include "support/LAssert.h"
#include "math_metricsinfo.h"
#include "frontends/Painter.h"
#include "textpainter.h"


using std::max;
using std::min;
using std::abs;



MathArray::MathArray()
	: xo_(0), yo_(0), clean_(false), drawn_(false)
{}


MathArray::MathArray(const_iterator from, const_iterator to)
	: base_type(from, to), xo_(0), yo_(0), clean_(false), drawn_(false)
{}


void MathArray::substitute(MathMacro const & m)
{
	for (iterator it = begin(); it != end(); ++it)
		it->nucleus()->substitute(m);
}


MathAtom & MathArray::operator[](size_type pos)
{
	lyx::Assert(pos < size());
	return base_type::operator[](pos);
}


MathAtom const & MathArray::operator[](size_type pos) const
{
	lyx::Assert(pos < size());
	return base_type::operator[](pos);
}


void MathArray::insert(size_type pos, MathAtom const & t)
{
	base_type::insert(begin() + pos, t);
}


void MathArray::insert(size_type pos, MathArray const & ar)
{
	base_type::insert(begin() + pos, ar.begin(), ar.end());
}


void MathArray::append(MathArray const & ar)
{
	insert(size(), ar);
}


void MathArray::erase(size_type pos)
{
	if (pos < size())
		erase(pos, pos + 1);
}


void MathArray::erase(iterator pos1, iterator pos2)
{
	base_type::erase(pos1, pos2);
}


void MathArray::erase(iterator pos)
{
	base_type::erase(pos);
}


void MathArray::erase(size_type pos1, size_type pos2)
{
	base_type::erase(begin() + pos1, begin() + pos2);
}


void MathArray::dump2() const
{
	NormalStream ns(lyxerr);
	for (const_iterator it = begin(); it != end(); ++it)
		ns << *it << ' ';
}


void MathArray::dump() const
{
	NormalStream ns(lyxerr);
	for (const_iterator it = begin(); it != end(); ++it)
		ns << "<" << *it << ">";
}


void MathArray::validate(LaTeXFeatures & features) const
{
	for (const_iterator it = begin(); it != end(); ++it)
		(*it)->validate(features);
}


bool MathArray::match(MathArray const & ar) const
{
	return size() == ar.size() && matchpart(ar, 0);
}


bool MathArray::matchpart(MathArray const & ar, pos_type pos) const
{
	if (size() < ar.size() + pos)
		return false;
	const_iterator it = begin() + pos;
	for (const_iterator jt = ar.begin(); jt != ar.end(); ++jt, ++it)
		if (!(*jt)->match(it->nucleus()))
			return false;
	return true;
}


void MathArray::replace(ReplaceData & rep)
{
	for (size_type i = 0; i < size(); ++i) {
		if (find1(rep.from, i)) {
			// match found
			lyxerr << "match found!\n";
			erase(i, i + rep.from.size());
			insert(i, rep.to);
		}
	}

#ifdef WITH_WARNINGS
#warning temporarily disabled
	// for (const_iterator it = begin(); it != end(); ++it)
	//	it->nucleus()->replace(rep);
#endif
}


bool MathArray::find1(MathArray const & ar, size_type pos) const
{
	//lyxerr << "finding '" << ar << "' in '" << *this << "'\n";
	for (size_type i = 0, n = ar.size(); i < n; ++i)
		if (!operator[](pos + i)->match(ar[i].nucleus()))
			return false;
	return true;
}


MathArray::size_type MathArray::find(MathArray const & ar) const
{
	for (int i = 0, last = size() - ar.size(); i < last; ++i)
		if (find1(ar, i))
			return i;
	return size();
}


MathArray::size_type MathArray::find_last(MathArray const & ar) const
{
	for (int i = size() - ar.size(); i >= 0; --i) 
		if (find1(ar, i))
			return i;
	return size();
}


bool MathArray::contains(MathArray const & ar) const
{
	if (find(ar) != size())
		return true;
	for (const_iterator it = begin(); it != end(); ++it)
		if (it->nucleus()->contains(ar))
			return true;
	return false;
}


void MathArray::touch() const
{
	clean_  = false;
	drawn_  = false;
}


Dimension const & MathArray::metrics(MathMetricsInfo & mi) const
{
	//if (clean_)
	//	return;
	clean_  = true;
	drawn_  = false;

	if (empty()) {
		mathed_char_dim(mi.base.font, 'I', dim_);
		return dim_;
	}

	dim_.clear();
	for (const_iterator it = begin(), et = end(); it != et; ++it) {
		(*it)->metrics(mi);
		dim_ += (*it)->dimensions();
	}
	return dim_;
}


void MathArray::draw(MathPainterInfo & pi, int x, int y) const
{
	//if (drawn_ && x == xo_ && y == yo_)
	//	return;
	//lyxerr << "MathArray::draw: x: " << x << " y: " << y << endl;

	xo_    = x;
	yo_    = y;
	drawn_ = true;

	if (y + descent() <= 0)                   // don't draw above the workarea
		return;
	if (y - ascent() >= pi.pain.paperHeight())   // don't draw below the workarea
		return;
	if (x + width() <= 0)                     // don't draw left of workarea
		return;
	if (x >= pi.pain.paperWidth())              // don't draw right of workarea
		return;

	if (empty()) {
		pi.pain.rectangle(x, y - ascent(), width(), height(), LColor::mathline);
		return;
	}

	for (const_iterator it = begin(), et = end(); it != et; ++it) {
		(*it)->draw(pi, x, y);
		x += (*it)->width();
	}
}


Dimension const & MathArray::metricsT(TextMetricsInfo const & mi) const
{
	//if (clean_)
	//	return;
	dim_.clear();
	for (const_iterator it = begin(); it != end(); ++it) {
		(*it)->metricsT(mi);
		dim_ += (*it)->dimensions();
	}
	return dim_;
}


void MathArray::drawT(TextPainter & pain, int x, int y) const
{
	//if (drawn_ && x == xo_ && y == yo_)
	//	return;
	//lyxerr << "x: " << x << " y: " << y << " " << pain.workAreaHeight() << endl;
	xo_    = x;
	yo_    = y;
	drawn_ = true;

	for (const_iterator it = begin(), et = end(); it != et; ++it) {
		(*it)->drawT(pain, x, y);
		x += (*it)->width();
	}
}


int MathArray::pos2x(size_type pos) const
{
	return pos2x(0, pos, 0);
}


int MathArray::pos2x(size_type pos1, size_type pos2, int glue) const
{
	int x = 0;
	size_type target = min(pos2, size());
	for (size_type i = pos1; i < target; ++i) {
		const_iterator it = begin() + i;
		MathInset const * p = it->nucleus();
		if (p->getChar() == ' ')
			x += glue;
		x += p->width();
	}
	return x;
}


MathArray::size_type MathArray::x2pos(int targetx) const
{
	return x2pos(0, targetx, 0);
}


MathArray::size_type MathArray::x2pos(size_type startpos, int targetx,
	int glue) const
{
	const_iterator it = begin() + startpos;
	int lastx = 0;
	int currx = 0;
	for (; currx < targetx && it < end(); ++it) {
		lastx = currx;
		MathInset const * p = it->nucleus();
		if (p->getChar() == ' ')
			currx += glue;
		currx += p->width();
	}
	if (abs(lastx - targetx) < abs(currx - targetx) && it != begin() + startpos)
		--it;
	return it - begin();
}


int MathArray::dist(int x, int y) const
{
	int xx = 0;
	int yy = 0;

	if (x < xo_)
		xx = xo_ - x;
	else if (x > xo_ + width())
		xx = x - xo_ - width();

	if (y < yo_ - ascent())
		yy = yo_ - ascent() - y;
	else if (y > yo_ + descent())
		yy = y - yo_ - descent();

	return xx + yy;
}


void MathArray::boundingBox(int & x1, int & x2, int & y1, int & y2)
{
	x1 = xo_;
	x2 = xo_ + width();
	y1 = yo_ - ascent();
	y2 = yo_ + descent();
}

void MathArray::center(int & x, int & y) const
{
	x = xo_ + width() / 2;
	y = yo_ + (descent() - ascent()) / 2;
}


void MathArray::towards(int & x, int & y) const
{
	int cx = 0;
	int cy = 0;
	center(cx, cy);

	double r = 1.0;
	//int dist = (x - cx) * (x - cx) + (y - cy) * (y - cy);

	x = cx + int(r * (x - cx));
	y = cy + int(r * (y - cy));
}


void MathArray::setXY(int x, int y)
{
	xo_ = x;
	yo_ = y;
}

