#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include "math_scriptinset.h"
#include "math_support.h"
#include "Painter.h"
#include "debug.h"


extern MathScriptInset const * asScript(MathArray::const_iterator it);


MathXArray::MathXArray()
	: width_(0), ascent_(0), descent_(0), xo_(0), yo_(0), size_()
{}


void MathXArray::metrics(MathMetricsInfo const & mi) const
{
	size_ = mi;
	mathed_char_dim(LM_TC_VAR, mi, 'I', ascent_, descent_, width_);

	if (data_.empty()) 
		return;

	math_font_max_dim(LM_TC_TEXTRM, mi, ascent_, descent_);	
	width_ = 0;

	//lyxerr << "MathXArray::metrics(): '" << data_ << "'\n";
	
	for (const_iterator it = begin(); it != end(); ++it) {
		MathInset const * p = it->nucleus();
		MathScriptInset const * q = (it + 1 == end()) ? 0 : asScript(it);
		if (q) {
			q->metrics(p, mi);
			ascent_  = std::max(ascent_,  q->ascent(p));
			descent_ = std::max(descent_, q->descent(p));
			width_  += q->width(p);	
			++it;
		} else {
			p->metrics(mi);
			ascent_  = std::max(ascent_,  p->ascent());
			descent_ = std::max(descent_, p->descent());
			width_  += p->width();	
		}
	}
	//lyxerr << "MathXArray::metrics(): '" << ascent_ << " " 
	//	<< descent_ << " " << width_ << "'\n";
}


void MathXArray::draw(Painter & pain, int x, int y) const
{
	xo_ = x;
	yo_ = y;

	if (data_.empty()) {
		pain.rectangle(x, y - ascent_, width_, height(), LColor::mathline);
		return;
	}

	for (const_iterator it = begin(); it != end(); ++it) {
		MathInset const * p = it->nucleus();
		MathScriptInset const * q = (it + 1 == end()) ? 0 : asScript(it);
		if (q) {
			q->draw(p, pain, x, y);
			x += q->width(p);
			++it;
		} else {
			p->draw(pain, x, y);
			x += p->width();
		}
	}
}


int MathXArray::pos2x(size_type targetpos) const
{
	int x = 0;
	const_iterator target = std::min(begin() + targetpos, end());
	for (const_iterator it = begin(); it < target; ++it) {
		MathInset const * p = it->nucleus();
		MathScriptInset const * q = (it + 1 == end()) ? 0 : asScript(it);
		if (q) {
			++it;
			if (it < target)
				x += q->width(p);
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
	for ( ; currx < targetx && it < end(); ++it) {
		lastx = currx;

		int wid = 0;
		MathInset const * p = it->nucleus();
		MathScriptInset const * q = 0;
		if (it + 1 != end())
			q = asScript(it);
		if (q) {
			wid = q->width(p);
			++it;
		} else
			wid = p->width();

		currx += wid;
	}
	if (abs(lastx - targetx) < abs(currx - targetx) && it != begin())
		--it;
	return it - begin();
}
