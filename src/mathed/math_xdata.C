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

	size_   = mi;
	clean_  = true;
	drawn_  = false;

	if (data_.empty()) {
		mathed_char_dim(mi.base.font, 'I', dim_);
		return dim_;;
	}

	dim_.clear();
	for (const_iterator it = begin(); it != end(); ++it) {
		MathInset const * p = it->nucleus();
		MathScriptInset const * q = (it + 1 == end()) ? 0 : asScript(it);
		Dimension d;
		if (q) {
			q->metrics(p, mi);
			q->dimensions2(p, d);
			++it;
		} else {
			p->metrics(mi);
			p->dimensions(d);
		}
		dim_ += d;
	}

	//lyxerr << "MathXArray::metrics(): '" << dim_ << "\n";
	return dim_;
}


void MathXArray::metricsExternal(MathMetricsInfo & mi,
	std::vector<Dimension> & v) const
{
	//if (clean_)
	//	return;

	size_   = mi;
	clean_  = true;
	drawn_  = false;

	if (data_.empty()) {
		mathed_char_dim(mi.base.font, 'I', dim_);
		return;
	}

	dim_.clear();
	for (const_iterator it = begin(); it != end(); ++it) {
		MathInset const * p = it->nucleus();
		MathScriptInset const * q = (it + 1 == end()) ? 0 : asScript(it);
		Dimension d;
		if (q) {
			q->metrics(p, mi);
			q->dimensions2(p, d);
			v.push_back(d);
			v.push_back(Dimension());
			++it;
		} else {
			p->metrics(mi);
			p->dimensions(d);
			v.push_back(d);
		}
	}

	//for (int i = 0; i < data_.size(); ++i)
	//	lyxerr << "i: " << i << "  dim: " << v[i] << endl;
	//lyxerr << "MathXArray::metrics(): '" << dim_ << "\n";
}


void MathXArray::draw(MathPainterInfo & pi, int x, int y) const
{
	//if (drawn_ && x == xo_ && y == yo_)
	//	return;

	//lyxerr << "x: " << x << " y: " << y << " " << pain.workAreaHeight() << endl;

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

	const_iterator it = begin(), et = end();

	if (it == et) {
		pi.pain.rectangle(x, y - ascent(), width(), height(), LColor::mathline);
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
}


void MathXArray::drawExternal(MathPainterInfo & pi, int x, int y,
	std::vector<Row> const & v) const
{
	//for (size_type r = 0; r < v.size(); ++r)
	//	lyxerr << "row " << r << " to: " << v[r].end << endl; 
	//lyxerr << " data: " << data_ << endl;

	xo_    = x;
	yo_    = y;

	for (size_type r = 0; r < v.size(); ++r) {
		int xx = x;
		int yy = y + v[r].yo;
		for (size_type pos = v[r].begin; pos < v[r].end && pos < data_.size(); ++pos) {
			//lyxerr << "drawing pos " << pos << " of " << data_.size() 
			//	<< " " << int(data_[pos]->getChar()) << endl;
			MathInset const * p = data_[pos].nucleus();
		
			// insert extra glue
			if (p->getChar() == ' ') 
				xx += v[r].glue;
	
			MathScriptInset const * q = 0;
			if (pos + 1 < data_.size())
				q = asScript(begin() + pos);
			if (q) {
				q->draw(p, pi, xx, yy);
				xx += q->width2(p);
				++pos;
			} else {
				p->draw(pi, xx, yy);
				xx += p->width();
			}
		}
	}
}


Dimension const & MathXArray::metricsT(TextMetricsInfo const & mi) const
{
	//if (clean_)
	//	return;
	dim_.clear();
	for (const_iterator it = begin(); it != end(); ++it) {
		MathInset const * p = it->nucleus();
		MathScriptInset const * q = (it + 1 == end()) ? 0 : asScript(it);
		Dimension d;
		if (q) {
			q->metricsT(p, mi);
			q->dimensions2(p, d);
			++it;
		} else {
			p->metricsT(mi);
			p->dimensions(d);
		}
		dim_ += d;
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


int MathXArray::pos2x(size_type pos) const
{
	return pos2x(0, pos, 0);
}

int MathXArray::pos2x(size_type pos1, size_type pos2, int glue) const
{
	int x = 0;
	size_type target = min(pos2, data_.size());
	for (size_type i = pos1; i < target; ++i) {
		const_iterator it = begin() + i;
		MathInset const * p = it->nucleus();
		if (p->getChar() == ' ')
			x += glue;
		MathScriptInset const * q = (i + 1 == data_.size()) ? 0 : asScript(it);
		if (q) {
			++i;
			if (i < target)
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
