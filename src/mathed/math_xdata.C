#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include "math_scriptinset.h"
#include "math_support.h"
#include "frontends/Painter.h"
#include "textpainter.h"
#include "debug.h"


using std::max;
using std::min;
using std::abs;


extern MathScriptInset const * asScript(MathArray::const_iterator it);


MathXArray::MathXArray()
	: width_(0), ascent_(0), descent_(0), xo_(0), yo_(0),
	  clean_(false), drawn_(false)
{}


void MathXArray::touch() const
{
	clean_  = false;
	drawn_  = false;
}


void MathXArray::metrics(MathMetricsInfo & mi) const
{
	//if (clean_)
	//	return;

	size_   = mi;
	clean_  = true;
	drawn_  = false;

	if (data_.empty()) {
		mathed_char_dim(mi.base.font, 'I', ascent_, descent_, width_);
		return;
	}

	width_   = 0;

	int a = 0;
	int d = 0;
	for (const_iterator it = begin(); it != end(); ++it) {
		MathInset const * p = it->nucleus();
		MathScriptInset const * q = (it + 1 == end()) ? 0 : asScript(it);
		int w = 0;
		if (q) {
			q->metrics(p, mi);
			a = max(a, q->ascent2(p));
			d = max(d, q->descent2(p));
			w = q->width2(p);
			++it;
		} else {
			p->metrics(mi);
			a = max(a, p->ascent());
			d = max(d, p->descent());
			w = p->width();
		}
		width_ += w;
	}

	ascent_  = a;
	descent_ = d;
	//width_   = 0;

	//lyxerr << "MathXArray::metrics(): '" << ascent_ << " "
	//	<< descent_ << " " << width_ << "'\n";


	//
	// re-break paragraph
	//
	if (mi.base.restrictwidth) {
		width_ = mi.base.textwidth;
		lyxerr << "restricting width to " << width_ << " pixel\n";
	}
}


void MathXArray::draw(MathPainterInfo & pi, int x, int y) const
{
	//if (drawn_ && x == xo_ && y == yo_)
	//	return;

	//lyxerr << "x: " << x << " y: " << y << " " << pain.workAreaHeight() << endl;

	xo_    = x;
	yo_    = y;
	drawn_ = true;

	if (y + descent_ <= 0)                   // don't draw above the workarea
		return;
	if (y - ascent_ >= pi.pain.paperHeight())   // don't draw below the workarea
		return;
	if (x + width_ <= 0)                     // don't draw left of workarea
		return;
	if (x >= pi.pain.paperWidth())              // don't draw right of workarea
		return;

	const_iterator it = begin(), et = end();

	if (it == et) {
		pi.pain.rectangle(x, y - ascent_, width_, height(), LColor::mathline);
		return;
	}

	for (; it != et; ++it) {
		MathInset const * p = it->nucleus();
		MathScriptInset const * q = (it + 1 == et) ? 0 : asScript(it);
		if (q) {
			q->draw(p, pi, x, y);
			x += q->width2(p);
			++it;
		} else {
			p->draw(pi, x, y);
			x += p->width();
		}
	}

	//
	// re-break paragraph
	//
	if (pi.base.restrictwidth) {
	}
}


void MathXArray::metricsT(TextMetricsInfo const & mi) const
{
	//if (clean_)
	//	return;

	ascent_  = 0;
	descent_ = 0;
	width_   = 0;

	for (const_iterator it = begin(); it != end(); ++it) {
		MathInset const * p = it->nucleus();
		MathScriptInset const * q = (it + 1 == end()) ? 0 : asScript(it);
		if (q) {
			q->metricsT(p, mi);
			ascent_  = max(ascent_,  q->ascent2(p));
			descent_ = max(descent_, q->descent2(p));
			width_  += q->width2(p);
			++it;
		} else {
			p->metricsT(mi);
			ascent_  = max(ascent_,  p->ascent());
			descent_ = max(descent_, p->descent());
			width_  += p->width();
		}
	}
}


void MathXArray::drawT(TextPainter & pain, int x, int y) const
{
	//if (drawn_ && x == xo_ && y == yo_)
	//	return;

	//lyxerr << "x: " << x << " y: " << y << " " << pain.workAreaHeight() << endl;

	xo_    = x;
	yo_    = y;
	drawn_ = true;

	const_iterator it = begin(), et = end();

	for (; it != et; ++it) {
		MathInset const * p = it->nucleus();
		MathScriptInset const * q = (it + 1 == et) ? 0 : asScript(it);
		if (q) {
			q->drawT(p, pain, x, y);
			x += q->width2(p);
			++it;
		} else {
			p->drawT(pain, x, y);
			x += p->width();
		}
	}
}


int MathXArray::pos2x(size_type targetpos) const
{
	int x = 0;
	const_iterator target = min(begin() + targetpos, end());
	for (const_iterator it = begin(); it < target; ++it) {
		MathInset const * p = it->nucleus();
		MathScriptInset const * q = (it + 1 == end()) ? 0 : asScript(it);
		if (q) {
			++it;
			if (it < target)
				x += q->width2(p);
			else  // "half" position
				x += q->dxx(p) + q->nwid(p);
		} else
			x += p->width();
	}
	return x;
}


MathArray::size_type MathXArray::x2pos(int targetx) const
{
	const_iterator it = begin();
	int lastx = 0;
	int currx = 0;
	for (; currx < targetx && it < end(); ++it) {
		lastx = currx;

		int wid = 0;
		MathInset const * p = it->nucleus();
		MathScriptInset const * q = 0;
		if (it + 1 != end())
			q = asScript(it);
		if (q) {
			wid = q->width2(p);
			++it;
		} else
			wid = p->width();

		currx += wid;
	}
	if (abs(lastx - targetx) < abs(currx - targetx) && it != begin())
		--it;
	return it - begin();
}


int MathXArray::dist(int x, int y) const
{
	int xx = 0;
	int yy = 0;

	if (x < xo_)
		xx = xo_ - x;
	else if (x > xo_ + width_)
		xx = x - xo_ - width_;

	if (y < yo_ - ascent_)
		yy = yo_ - ascent_ - y;
	else if (y > yo_ + descent_)
		yy = y - yo_ - descent_;

	return xx + yy;
}


void MathXArray::boundingBox(int & x1, int & x2, int & y1, int & y2)
{
	x1 = xo_;
	x2 = xo_ + width_;
	y1 = yo_ - ascent_;
	y2 = yo_ + descent_;
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

		// move on
		MathScriptInset const * q = (it + 1 == end()) ? 0 : asScript(it);
		if (q) {
			x += q->width(p);
			f.nextPos();
			++it;
		} else {
			x += p->width();
		}
	}
}
*/

void MathXArray::center(int & x, int & y) const
{
	x = xo_ + width_ / 2;
	y = yo_ + (descent_ - ascent_) / 2;
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
