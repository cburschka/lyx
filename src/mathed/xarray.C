#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include "xarray.h"
#include "math_inset.h"
#include "math_scriptinset.h"
#include "mathed/support.h"
#include "math_defs.h"
#include "Painter.h"
#include "debug.h"


MathXArray::MathXArray()
	: width_(0), ascent_(0), descent_(0), xo_(0), yo_(0), style_(LM_ST_TEXT)
{}


void MathXArray::metrics(MathStyles st) const
{
	style_   = st;
	mathed_char_dim(LM_TC_VAR, st, 'I', ascent_, descent_, width_);

	if (data_.empty()) 
		return;

	math_font_max_dim(LM_TC_TEXTRM, st, ascent_, descent_);	
	width_   = 0;

	//lyxerr << "MathXArray::metrics(): '" << data_ << "'\n";
	
	for (const_iterator it = begin(); it != end(); ++it) {
		MathInset const * p = it->nucleus();
		if (MathScriptInset const * q = data_.asScript(it)) {
			q->metrics(p, st);
			ascent_  = std::max(ascent_,  q->ascent(p));
			descent_ = std::max(descent_, q->descent(p));
			width_  += q->width(p);	
			++it;
		} else {
			p->metrics(st);
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
		if (MathScriptInset const * q = data_.asScript(it)) {
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
		if (MathScriptInset const * q = data_.asScript(it)) {
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
		if (MathScriptInset const * q = data_.asScript(it)) {
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


std::ostream & operator<<(std::ostream & os, MathXArray const & ar)
{
	os << ar.data_;
	return os;
}
