#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include "math_xdata.h"
#include "math_metricsinfo.h"
#include "math_support.h"
#include "math_inset.h"
#include "frontends/Painter.h"
#include "textpainter.h"
#include "debug.h"


using std::max;
using std::min;
using std::abs;


MathXArray::MathXArray()
	: xo_(0), yo_(0), clean_(false), drawn_(false)
{}


void MathXArray::touch() const
{
	clean_  = false;
	drawn_  = false;
}


Dimension const & MathXArray::metrics(MathMetricsInfo & mi) const
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


void MathXArray::draw(MathPainterInfo & pi, int x, int y) const
{
	//if (drawn_ && x == xo_ && y == yo_)
	//	return;
	//lyxerr << "MathXArray::draw: x: " << x << " y: " << y << endl;

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


Dimension const & MathXArray::metricsT(TextMetricsInfo const & mi) const
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


void MathXArray::drawT(TextPainter & pain, int x, int y) const
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


int MathXArray::pos2x(size_type pos) const
{
	return pos2x(0, pos, 0);
}


int MathXArray::pos2x(size_type pos1, size_type pos2, int glue) const
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


MathArray::size_type MathXArray::x2pos(int targetx) const
{
	return x2pos(0, targetx, 0);
}


MathArray::size_type MathXArray::x2pos(size_type startpos, int targetx,
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


int MathXArray::dist(int x, int y) const
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


void MathXArray::boundingBox(int & x1, int & x2, int & y1, int & y2)
{
	x1 = xo_;
	x2 = xo_ + width();
	y1 = yo_ - ascent();
	y2 = yo_ + descent();
}

/*
void MathXArray::findPos(MathPosFinder & f) const
{
	double x = xo_;
	double y = yo_;
	for (const_iterator it = begin(); it < end(); ++it) {
		// check this position in the cell first
		f.visit(x, y);
		f.nextPos();

		// check inset
		MathInset const * p = it->nucleus();
		p->findPos(f);
		x += p->width();
	}
}
*/

void MathXArray::center(int & x, int & y) const
{
	x = xo_ + width() / 2;
	y = yo_ + (descent() - ascent()) / 2;
}


void MathXArray::towards(int & x, int & y) const
{
	int cx = 0;
	int cy = 0;
	center(cx, cy);

	double r = 1.0;
	//int dist = (x - cx) * (x - cx) + (y - cy) * (y - cy);

	x = cx + int(r * (x - cx));
	y = cy + int(r * (y - cy));
}


std::ostream & operator<<(std::ostream & os, MathXArray const & ar)
{
	os << ar.data();
}

