#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include "xarray.h"
#include "math_inset.h"
#include "mathed/support.h"
#include "math_defs.h"
#include "Painter.h"
#include "debug.h"

using std::max;
using std::min;


MathXArray::MathXArray()
	: width_(0), ascent_(0), descent_(0), xo_(0), yo_(0), style_(LM_ST_TEXT)
{}


void MathXArray::metrics(MathStyles st) const
{
	style_   = st;
	mathed_char_dim(LM_TC_VAR, st, 'I', ascent_, descent_, width_);

	if (data_.empty()) 
		return;

	math_font_max_dim(LM_TC_TEXTRM, LM_ST_TEXT, ascent_, descent_);	
	width_   = 0;

	//lyxerr << "MathXArray::metrics(): '" << data_ << "'\n";
	for (int pos = 0; pos < data_.size(); ++pos) {
		MathInset const * p = data_.nextInset(pos);
		p->metrics(st);
		int asc  = p->ascent();
		int des  = p->descent();
		int wid  = p->width();
		ascent_  = max(ascent_, asc);
		descent_ = max(descent_, des);
		width_   += wid;
	}
}


void MathXArray::draw(Painter & pain, int x, int y) const
{
	xo_ = x;
	yo_ = y;

	if (data_.empty()) {
		pain.rectangle(x, y - ascent_, width_, height(), LColor::mathline);
		return;
	}

	for (int pos = 0; pos < data_.size(); ++pos) {
		MathInset const * p = data_.nextInset(pos);
		p->draw(pain, x, y);
		x += p->width();
	}
}


int MathXArray::pos2x(int targetpos) const
{
	int x = 0;
	targetpos = min(targetpos, data_.size());
	for (int pos = 0; pos < targetpos; ++pos) 
		x += width(pos);
	return x;
}


int MathXArray::x2pos(int targetx) const
{
	int pos   = 0;
	int lastx = 0;
	int currx = 0;
	for ( ; currx < targetx && pos < data_.size(); ++pos) {
		lastx = currx;
		currx += width(pos);
	}
	if (abs(lastx - targetx) < abs(currx - targetx) && pos > 0)
		--pos;
	return pos;
}


int MathXArray::width(int pos) const
{
	if (pos >= data_.size())
		return 0;
	return data_.nextInset(pos)->width();
}


std::ostream & operator<<(std::ostream & os, MathXArray const & ar)
{
	os << ar.data_;
	return os;
}

